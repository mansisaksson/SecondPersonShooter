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

			if (PossessedEnemy == NULL)
			{
				PossessedEnemy = DefaultGameMode->GetNextEnemy();
				PossessEnemy(PossessedEnemy);
			}

			// Fire Weapon Stuff
			TimeSinceFire += DeltaSeconds;
			if (bIsFiring)
			{
				if (TimeSinceFire > 1.f / ShotsPerSecond)
				{
					TimeSinceFire = 0.f;
					FireWeapon();
				}
			}
		}
		else
		{
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
		}

		// Rotate Player Stuff
		if (PossessedEnemy != NULL)
		{
			if ((xTurnRate * xTurnRate) + (yTurnRate * yTurnRate) > 0.5f)
			{
				
				//relative enemy direction
				//FVector InputVector(-xTurnRate, yTurnRate, 0.f);
				//RelativeInputRotation = PossessedEnemy->GetTransform().TransformVectorNoScale(InputVector);

				//relative direction between enemy and player
				FVector InputVector(xTurnRate, -yTurnRate, 0.f);
				FVector DirectionVec = PossessedEnemy->GetTransform().GetLocation()-GetTransform().GetLocation();
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
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (PossessedEnemy != NULL)
			{
				const FRotator Rotation = PossessedEnemy->GetController()->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);
			}
		}
	}
}
void APlayerCharacter::MoveRight(float Value)
{
	if (bIsDead == false)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (PossessedEnemy != NULL)
			{
				const FRotator Rotation = PossessedEnemy->GetController()->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, Value);
			}
		}
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
		ECollisionChannel collisionChannel;
		collisionChannel = ECC_WorldDynamic;
		FCollisionQueryParams collisionQuery;
		collisionQuery.bTraceComplex = true;
		FCollisionObjectQueryParams objectCollisionQuery;
		objectCollisionQuery = FCollisionObjectQueryParams::DefaultObjectQueryParam;
		FCollisionResponseParams collisionResponse;
		collisionResponse = ECR_Block;
		collisionQuery.AddIgnoredActor(this);

		bool hitObject = GetWorld()->LineTraceSingleByChannel(result, BulletSpawnComp->GetComponentLocation(), TowardsLocation, collisionChannel, collisionQuery, collisionResponse);

		if (hitObject && result.Actor != NULL)
		{
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
		}
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
	Borde funka att bara anv�nda: (har inte testat)
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