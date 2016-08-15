#include "SecondPersonShooter.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "DefaultGameMode.h"
#include "Engine.h"


APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	xTurnRate = 0.f;
	yTurnRate = 0.f;
	ShotsPerSecond = 3.f;
	TurnRate = 10.f;
	bIsFiring = false;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Camera->bUsePawnControlRotation = false;

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("GunHand"));

	BulletSpawnComp = CreateDefaultSubobject<USceneComponent>(TEXT("BulletSpawnLocation"));
	BulletSpawnComp->AttachToComponent(GunMesh, FAttachmentTransformRules::KeepRelativeTransform);

	FadeDarkness = 0;
	FadeResetSpeed = 0;
	FadeResetDelay = 0;
	FadeTime = 0;
	FadeMax = 0;
	FadeMin = 0;
	FadedTime = 0;
	FadeRed = false;

	TVFadeValue = 0;
	TVFadeResetSpeed = 0;
	TVFadeResetDelay = 0;
	TVFadeTime = 0;
	TVFadeMax = 0;
	TVFadeMin = 0;
	TVFadedTime = 0;
	
	Health = 100.f;
	MaxHealth = 100.f;
	Special = 100.f;
	MaxSpecial = 100.f;
	shieldTime = 0;
	SuperSpeedTime = 0;
	AttackSpeedBonus = 1.0;
	SuperWeaponSpeed = 1.0;
	weapon = EWeaponType::StarterWeapon;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == NULL)
		UE_LOG(LogTemp, Warning, TEXT("FAILED TO LOAD PLAYER CONTROLLER!"));

	DefaultGameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
	if (DefaultGameMode == NULL)
		UE_LOG(LogTemp, Warning, TEXT("NO DEFAULT GAME MODE FOUND!"));

	if (PossessedEnemy == NULL)
	{
		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			AEnemyCharacter* enemy = Cast<AEnemyCharacter>(*ActorItr);
			if (enemy != NULL && enemy->StartOnThis)
			{
				PossessedEnemy = enemy;
				PossessEnemy(enemy);
			}
		}
	}

	DefaultMaxMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	bool gameRunning = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode())->IsGameplayRunning();

	if (!bIsDead)
	{
		if (gameRunning)
		{
			if (shieldTime > 0)
				shieldTime -= DeltaSeconds;

			UpdatePowerups(DeltaSeconds);

			if (PossessedEnemy == NULL)
			{
				PossessedEnemy = DefaultGameMode->GetNextEnemy();
				PossessEnemy(PossessedEnemy);
			}
			else if (PossessedEnemy->IsPendingKillPending() || !PossessedEnemy->GetIsAlive())
			{
				PossessedEnemy = DefaultGameMode->GetNextEnemy();
				PossessEnemy(PossessedEnemy);
			}

			// Fire Weapon Stuff
			TimeSinceFire += DeltaSeconds;
			if (bIsFiring || weapon == EWeaponType::Laser)
			{
				if (TimeSinceFire > 1.f / (ShotsPerSecond * AttackSpeedBonus * SuperWeaponSpeed))
				{
					TimeSinceFire = 0.f;
					FireWeapon();
				}
			}
			

			if (PossessedEnemy != NULL)
			{
				FVector StartPositon = PossessedEnemy->GetCamera()->GetComponentLocation();
				FVector EndPositon = StartPositon + (PossessedEnemy->GetCamera()->GetForwardVector() * 50000.f);

				FHitResult result;
				FCollisionQueryParams collisionQuery;
				collisionQuery.bTraceComplex = false;
				collisionQuery.AddIgnoredActor(this);

				GetWorld()->LineTraceSingleByChannel(result, StartPositon, EndPositon, ECC_WorldDynamic, collisionQuery, ECR_Block);

				if (AEnemyCharacter* enemy = Cast<AEnemyCharacter>(result.GetActor()))
				{
					if (HighlightedEnemy != enemy)
					{
						if (HighlightedEnemy != NULL)
							HighlightedEnemy->SetEnemyHighlighted(false);
						enemy->SetEnemyHighlighted(true);
						HighlightedEnemy = enemy;
					}
				}
				else if (HighlightedEnemy != NULL)
				{
					HighlightedEnemy->SetEnemyHighlighted(false);
					HighlightedEnemy = NULL;
				}
			}
		}

		if (Controller != NULL && PossessedEnemy != NULL)
		{
			// Direction relative possessed enemy, used for movement and rotation
			FVector DirectionVec = PossessedEnemy->GetTransform().GetLocation() - GetTransform().GetLocation();
			FVector InputVector;

			// Move Player Stuff
			FVector2D speed = FVector2D(xMoveDirection, yMoveDirection);
			if (speed.Size() > 0.3)
			{
				if (speed.Size() > 0.9)
				{
					speed.Normalize();
					xMoveDirection = speed.X;
					yMoveDirection = speed.Y;
				}
				InputVector = FVector(-xMoveDirection, -yMoveDirection, 0.f);
				RelativeInputRotation = DirectionVec.Rotation().RotateVector(InputVector);

				AddMovementInput(RelativeInputRotation, RelativeInputRotation.Size());
			}

			// Rotate Player Stuff
			FVector2D direction = FVector2D(xTurnRate, yTurnRate);
			if (direction.Size() > 0.3)
			{
				if (direction.Size() > 0.9)
				{
					direction.Normalize();
					xTurnRate = direction.X;
					yTurnRate = direction.Y;
				}
				//relative enemy direction
				//FVector InputVector(-xTurnRate, yTurnRate, 0.f);
				//RelativeInputRotation = PossessedEnemy->GetTransform().TransformVectorNoScale(InputVector);

				//relative direction between enemy and player
				InputVector = FVector(xTurnRate, -yTurnRate, 0.f);
				RelativeInputRotation = DirectionVec.Rotation().RotateVector(InputVector);

				//smooth
				PlayerController->SetControlRotation(FMath::RInterpTo(GetActorRotation(), RelativeInputRotation.Rotation(), DeltaSeconds, TurnRate));
				//no smooth
				//PlayerController->SetControlRotation(RelativeInputRotation.Rotation());
			}
		}
	}
	

	// Fade Stuff
	if (FadeTime > 0)
	{
		FadeDarkness = FMath::Lerp(FadeMin, FadeMax, FadedTime); // lerp(fademax, fademin, FadedTime)
		FadedTime += DeltaSeconds / FadeTime;
		if (FadedTime >= FadeTime)
		{
			FadeTime = 0;
			FadeDarkness = FadeMax;
		}
	}
	else if (FadeDarkness > 0)
	{
		if (FadeResetSpeed > 0)
		{
			FadeResetDelay -= DeltaSeconds;
			if (FadeResetDelay <= 0)
			{
				FadeResetDelay = 0;
				FadeDarkness -= FadeResetSpeed * DeltaSeconds;
			}
		}
	}
	else FadeDarkness = 0;

	if (TVFadeTime > 0)
	{
		TVFadeValue = FMath::Lerp(TVFadeMin, TVFadeMax, TVFadedTime); // lerp(fademax, fademin, FadedTime)
		TVFadedTime += DeltaSeconds / TVFadeTime;
		if (TVFadedTime >= TVFadeTime)
		{
			TVFadeTime = 0;
			TVFadeValue = TVFadeMax;
		}
	}
	else if (TVFadeValue > 0)
	{
		if (TVFadeResetSpeed > 0)
		{
			TVFadeResetDelay -= DeltaSeconds;
			if (TVFadeResetDelay <= 0)
			{
				TVFadeResetDelay = 0;
				TVFadeValue -= TVFadeResetSpeed * DeltaSeconds;
			}
		}
	}
	else TVFadeValue = 0;
}

void APlayerCharacter::UpdatePowerups(float DeltaSeconds)
{
	//IsPendingKill();
	if(SuperSpeedTime > 0)
	{
		SuperSpeedTime -= DeltaSeconds;
	}	
	else
	{
		SuperSpeedTime = 0;
		AttackSpeedBonus = 1.0;
		GetCharacterMovement()->MaxWalkSpeed = DefaultMaxMoveSpeed;
	}	
	
	if(SuperWeaponTime > 0)
	{
		SuperWeaponTime -= DeltaSeconds;
	}	
	else
	{
		SuperWeaponSpeed = 1.0;
		SuperWeaponTime = 0;
		weapon = EWeaponType::StarterWeapon;
	}	
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	AEnemyCharacter* enemy = Cast<AEnemyCharacter>(DamageCauser);
	if (shieldTime <= 0 && bIsDead == false)
	{
		Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
		shieldTime = 1.0f;

		if (Health <= 0.f)
		{
			FadeMax = 1;
			FadedTime = 0;
			FadeTime = 2.0;
			FadeMin = 0.2;
			FadeResetSpeed = 0;
			FadeRed = true;
			bIsDead = true;

			SPS::GetGameMode(this)->OnPlayerDeath();

			GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldDynamic);
		}
		else
		{
			FadedTime = 0;
			FadeMax = 0.7;
			FadeTime = 0.3;
			FadeMin = 0.0;
			FadeResetSpeed = 0.3;
			FadeResetDelay = 0.4;
			FadeRed = true;
		}
	}

	return Health;
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("SwapRight", IE_Released, this, &APlayerCharacter::SwapCloser_Input);
	InputComponent->BindAction("SwapLeft", IE_Released, this, &APlayerCharacter::SwapFurther_Input);

	InputComponent->BindAction("FireWeapon", IE_Pressed, this, &APlayerCharacter::StartFire);
	InputComponent->BindAction("FireWeapon", IE_Released, this, &APlayerCharacter::StopFire);

	InputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	InputComponent->BindAxis("FaceUp", this, &APlayerCharacter::FaceUp);
	InputComponent->BindAxis("FaceRight", this, &APlayerCharacter::FaceRight);

	InputComponent->BindAction("Restart", IE_Released, this, &APlayerCharacter::RestartGame);
	InputComponent->BindAction("Exit", IE_Released, this, &APlayerCharacter::ExitGame);
}

void APlayerCharacter::RestartGame()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
void APlayerCharacter::ExitGame()
{
	GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
}

void APlayerCharacter::MoveForward(float Value)
{
	if (bIsDead == false)
	{
		xMoveDirection = Value;
	}
}
void APlayerCharacter::MoveRight(float Value)
{
	if (bIsDead == false)
	{
		yMoveDirection = Value;
	}
}

void APlayerCharacter::FaceUp(float Value)
{
	if (bIsDead == false)
		xTurnRate = Value;
}
void APlayerCharacter::FaceRight(float Value)
{
	if (bIsDead == false)
		yTurnRate = Value;
}

void APlayerCharacter::SwapCloser_Input()
{
	if (SPS::GetGameMode(this)->IsGameplayRunning())
		SwapCloser();
}
void APlayerCharacter::SwapFurther_Input()
{
	if (SPS::GetGameMode(this)->IsGameplayRunning())
		SwapFurther();
}
void APlayerCharacter::SwapRandom_Input()
{
	if (SPS::GetGameMode(this)->IsGameplayRunning())
		SwapRandom();
}

void APlayerCharacter::SwapCloser()
{
	if (bIsDead == false)
	{
		if (StaticSound != NULL)
			UGameplayStatics::PlaySoundAtLocation(this, StaticSound, GetActorLocation());

		AEnemyCharacter* TempEnemy = NULL;
		if (PossessedEnemy != NULL)
			TempEnemy = DefaultGameMode->GetCloserEnemy(PossessedEnemy);
		else
			TempEnemy = DefaultGameMode->GetNextEnemy();

		if (TempEnemy != NULL)
		{
			PossessedEnemy = TempEnemy;
			PossessEnemy(PossessedEnemy);
		}
	}
}
void APlayerCharacter::SwapFurther()
{
	if (bIsDead == false)
	{
		if (StaticSound != NULL)
			UGameplayStatics::PlaySoundAtLocation(this, StaticSound, GetActorLocation());

		AEnemyCharacter* TempEnemy = NULL;
		if (PossessedEnemy != NULL)
			TempEnemy = DefaultGameMode->GetFurtherEnemy(PossessedEnemy);
		else
			TempEnemy = DefaultGameMode->GetPrevEnemy();

		if (TempEnemy != NULL)
		{
			PossessedEnemy = TempEnemy;
			PossessEnemy(PossessedEnemy);
		}
	}
}
void APlayerCharacter::SwapRandom()
{
	AEnemyCharacter* TempEnemy = NULL;
	if (PossessedEnemy != NULL)
		TempEnemy = DefaultGameMode->GetRandomEnemy();

	if (TempEnemy != NULL)
	{
		PossessedEnemy = TempEnemy;
		PossessEnemy(PossessedEnemy);
	}
}
void APlayerCharacter::PossessEnemy(class AEnemyCharacter* Enemy)
{
	if (bIsDead == false)
	{
		if (Enemy != NULL)
		{
			TVFadedTime = 0;
			TVFadeMax = 0.2;
			TVFadeTime = 0.2;
			TVFadeMin = 0.2;
			TVFadeResetSpeed = 0.2;
			TVFadeResetDelay = 0.2;
			TVFadeValue = FMath::Lerp(TVFadeMin, TVFadeMax, TVFadedTime);
			PlayerController->SetViewTargetWithBlend(Enemy);
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Tried To Swap To NULL Enemy!"));
	}
}

void APlayerCharacter::StartFire()
{
	ADefaultGameMode* gameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	if (gameMode->IsGameLoaded())
	{
		if (!gameMode->IsGameplayRunning())
			gameMode->StartGameplay();

		if (bIsDead == false)
			bIsFiring = true;
	}
}
void APlayerCharacter::StopFire()
{
	bIsFiring = false;
}
void APlayerCharacter::FireWeapon()
{
	if (bIsDead == false)
	{
		OnFire();
		
		if( weapon == EWeaponType::StarterWeapon )
			FireNormalWeapon();
		//else if ( weapon == EWeaponType::Shotgun )
		//	;
		else if (weapon == EWeaponType::Laser)
			FireLaserWeapon();
			
	}
}

void APlayerCharacter::FireNormalWeapon()
{
	if (FireSound != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());

	if (MuzzeFlash != NULL)
	{
		UParticleSystemComponent* particleComp = UGameplayStatics::SpawnEmitterAttached(MuzzeFlash, BulletSpawnComp);
		FTransform particleTransform = particleComp->GetRelativeTransform();
		particleTransform.SetScale3D(FVector(0.1f, 0.1f, 0.1f));
		particleComp->SetRelativeTransform(particleTransform);
	}

	FVector TowardsLocation = BulletSpawnComp->GetComponentLocation() + (GetCapsuleComponent()->GetComponentRotation().Vector() * 50000.f);

	FHitResult result;
	FCollisionQueryParams collisionQuery;
	collisionQuery.bTraceComplex = true;
	collisionQuery.AddIgnoredActor(this);

	bool hitObject = GetWorld()->LineTraceSingleByChannel(result, BulletSpawnComp->GetComponentLocation(), TowardsLocation, ECC_WorldDynamic, collisionQuery, ECR_Block);

	if (hitObject && result.Actor != NULL)
	{
	/*	if(PossessedEnemy == result.Actor)
		{
			
			if (HitSparks != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitSparks, result.Location, FRotator::ZeroRotator, true);

			if (ShieldHit != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShieldHit, result.Location, FRotator::ZeroRotator, true);

		}
		else
		{
		*/	
			if (HitSparks != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitSparks, result.Location, FRotator::ZeroRotator, true);

			if (TrailParticle != NULL)
			{
				UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailParticle, BulletSpawnComp->GetComponentLocation(), FRotator::ZeroRotator, true);
				ParticleComp->SetBeamEndPoint(0, result.Location);
			}

			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);
			result.Actor->TakeDamage(FMath::RandRange(40.f, 60.f), DamageEvent, GetController(), this);
		//}
	}
}

void APlayerCharacter::FireLaserWeapon()
{
	//if (FireSound != NULL)
	//	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());

	if (LaserParticle != NULL)
	{
		UParticleSystemComponent* particleComp = UGameplayStatics::SpawnEmitterAttached(LaserParticle, BulletSpawnComp);
		FTransform particleTransform = particleComp->GetRelativeTransform();
		particleTransform.SetScale3D(FVector(0.35f, 0.35f, 0.35f));
		particleComp->SetRelativeTransform(particleTransform);
	}

	FCollisionQueryParams collisionQuery;
	collisionQuery.bTraceComplex = true;
	collisionQuery.AddIgnoredActor(this);
	
	FVector TowardsLocation = BulletSpawnComp->GetComponentLocation() + (GetCapsuleComponent()->GetComponentRotation().Vector() * 50000.f);
	FVector TowardsLocation2 = BulletSpawnComp->GetComponentLocation() + BulletSpawnComp->GetRightVector() * 50 + (GetCapsuleComponent()->GetComponentRotation().Vector() * 50000.f);
	FVector TowardsLocation3 = BulletSpawnComp->GetComponentLocation() + BulletSpawnComp->GetRightVector() * -50 + (GetCapsuleComponent()->GetComponentRotation().Vector() * 50000.f);
	FHitResult result;
	FHitResult result2;
	FHitResult result3;

	bool hitObject = GetWorld()->LineTraceSingleByChannel(result, BulletSpawnComp->GetComponentLocation(), TowardsLocation, ECC_WorldDynamic, collisionQuery, ECR_Block);
	hitObject = hitObject || GetWorld()->LineTraceSingleByChannel(result2, BulletSpawnComp->GetComponentLocation()+ BulletSpawnComp->GetRightVector() * 50, TowardsLocation2, ECC_WorldDynamic, collisionQuery, ECR_Block);
	hitObject = hitObject || GetWorld()->LineTraceSingleByChannel(result3, BulletSpawnComp->GetComponentLocation()+ BulletSpawnComp->GetRightVector() * -50, TowardsLocation3, ECC_WorldDynamic, collisionQuery, ECR_Block);
	
	TSet<AActor*> Actors;
	if(hitObject)
	{
		//spawn only 1 particleeffect
		if (result.Actor != NULL)
		{
			Actors.Add(result.GetActor());
			if (LaserHit != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaserHit, result.Location, FRotator::ZeroRotator, true);
		}
		else if(result2.Actor != NULL)
		{
			if (LaserHit != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaserHit, result2.Location, FRotator::ZeroRotator, true);
		}
		else if(result3.Actor != NULL)
		{
			if (LaserHit != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LaserHit, result3.Location, FRotator::ZeroRotator, true);
		}
		// add all hit enemies in a set to not get duplicates
		if(result2.Actor != NULL)
		{
			Actors.Add(result2.GetActor());
		}
		if(result3.Actor != NULL)
		{
			Actors.Add(result3.GetActor());
		}
		
	}
	// 
	TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	FDamageEvent DamageEvent(ValidDamageTypeClass);
	for (AActor* Actor : Actors)
	{
		Actor->TakeDamage(FMath::RandRange(25.f, 35.f), DamageEvent, GetController(), this);
	}

}

void APlayerCharacter::PossessedIsKilled()
{
	// StaticNoiseBig();
	if (StaticSound != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, StaticSound, GetActorLocation());

	SwapRandom();
	TVFadedTime = 0;
	TVFadeMin = 0.5;
	TVFadeMax = 0.5;
	TVFadeTime = 0.2;
	TVFadeResetSpeed = 0.4;
	TVFadeResetDelay = 0.3;
	/*
	Borde funka att bara använda: (har inte testat)
	TVFadeValue = x;
	TVFadeResetSpeed = y;
	TVFadeResetDelay = z;
	*/
}

void APlayerCharacter::PossessedIsDamaged()
{
	if (StaticSound != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, StaticSound, GetActorLocation());

	TVFadedTime = 0;
	TVFadeMin = 0.4;
	TVFadeMax = 0.4;
	TVFadeTime = 0.1;
	TVFadeResetSpeed = 0.2;
	TVFadeResetDelay = 0.0;
}

float APlayerCharacter::GetRotationFromEnemy()
{
	return FMath::Atan2(FMath::Sin(RelativeInputRotation.Y), FMath::Cos(RelativeInputRotation.X));
}

void APlayerCharacter::OnFire_Implementation()
{

}