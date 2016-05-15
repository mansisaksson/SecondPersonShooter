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
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	bIsFiring = false;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APlayerCharacter::OnHit);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachParent = GetCapsuleComponent();
	Camera->bUsePawnControlRotation = false;

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->AttachTo(GetMesh(), TEXT("GunHand"), EAttachLocation::SnapToTargetIncludingScale, true);

	BulletSpawnComp = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Origin"));
	BulletSpawnComp->AttachTo(GetMesh());

	
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
	hp = 2;
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

	TimeInEnemy += DeltaSeconds;
	if (TimeInEnemy > 1.f)
	{
		if (PossessedEnemy != NULL)
			PossessedEnemy->GetCharacterMovement()->MaxWalkSpeed += 0.15f;
	}

	if (shieldTime > 0)
		shieldTime -= DeltaSeconds;

	if (PossessedEnemy == NULL)
	{
		if (PossessedEnemy != NULL)
			PossessedEnemy->GetCharacterMovement()->MaxWalkSpeed = PossessedEnemy->DefaultWalkSpeed;

		PossessedEnemy = DefaultGameMode->GetNextEnemy();
		Swap(PossessedEnemy);
	}

	// Rotate Player Stuff
	else
	{
		if (!(xTurnRate == 0.f && yTurnRate == 0.f))
		{
			FVector InputVector(-xTurnRate, yTurnRate, 0.f);
			FVector NewVector = PossessedEnemy->GetTransform().TransformVectorNoScale(InputVector);
			
			PlayerController->SetControlRotation(FMath::Lerp(GetActorRotation(), NewVector.Rotation(), 20.f * DeltaSeconds));

			xTurnRate = GetControlRotation().Vector().X;
			xTurnRate = GetControlRotation().Vector().Y;
		}
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

	// Fade Stuff
	if (FadeTime > 0)
	{
		FadeDarkness = FMath::Lerp(FadeMin, FadeMax, FadedTime); // lerp(fademax, fademin, FadedTime)
		FadedTime += DeltaSeconds/FadeTime;
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

void APlayerCharacter::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AEnemyCharacter *enemy = Cast<AEnemyCharacter>(OtherActor);
	if (shieldTime <= 0)
		if (enemy && dead == false)
		{
			//PossessedEnemy = enemy;
			//PlayerController->SetViewTargetWithBlend(enemy);
			hp--;
			shieldTime = 1.0f;
			if (hp < 0)
			{
				FadeMax = 1;
				FadedTime = 0;
				FadeTime = 2.0;
				FadeMin = 0.2;
				FadeResetSpeed = 0;
				FadeRed = true;
				dead = true;

				GetMesh()->SetSimulatePhysics(true);
				GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
				GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
				GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);

				GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
				GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
				GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldDynamic);
			}
			else
			{
				//PossessedEnemy = enemy;
				//PlayerController->SetViewTargetWithBlend(enemy);

				FadedTime = 0;
				FadeMax = 0.7;
				FadeTime = 0.3;
				FadeMin = 0.0;
				FadeResetSpeed = 0.3;
				FadeResetDelay = 0.4;
				FadeRed = true;
			}
		}
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("SwapRight", IE_Released, this, &APlayerCharacter::SwapRight);
	InputComponent->BindAction("SwapLeft", IE_Released, this, &APlayerCharacter::SwapLeft);
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
	if (dead == false)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (PossessedEnemy != NULL)
			{
				const FRotator Rotation = PossessedEnemy->GetController()->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);

				/*const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);*/
			}
		}
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (dead == false)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			if (PossessedEnemy != NULL)
			{
				const FRotator Rotation = PossessedEnemy->GetController()->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, Value);

				/*const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, Value);*/
			}
		}
	}
}

void APlayerCharacter::FaceUp(float Value)
{
	if (dead == false)
	{
		//if (Value != 0.f)
			xTurnRate = Value;
	}
}

void APlayerCharacter::FaceRight(float Value)
{
	if (dead == false)
	{
		//if (Value != 0.f)
			yTurnRate = Value;
	}
}

void APlayerCharacter::SwapRight()
{
	if (dead == false)
	{
		if (PossessedEnemy != NULL)
			PossessedEnemy->GetCharacterMovement()->MaxWalkSpeed = PossessedEnemy->DefaultWalkSpeed;

		AEnemyCharacter* TempEnemy = DefaultGameMode->GetNextEnemy();
		if (TempEnemy != NULL)
		{
			PossessedEnemy = TempEnemy;
			Swap(PossessedEnemy);
		}
	}
}

void APlayerCharacter::SwapLeft()
{
	if (dead == false)
	{
		if (PossessedEnemy != NULL)
			PossessedEnemy->GetCharacterMovement()->MaxWalkSpeed = PossessedEnemy->DefaultWalkSpeed;

		AEnemyCharacter* TempEnemy = DefaultGameMode->GetPrevEnemy();
		if (TempEnemy != NULL)
		{
			PossessedEnemy = TempEnemy;
			Swap(PossessedEnemy);
		}
	}
}

void APlayerCharacter::StartFire()
{
	if (dead == false)
	{
		bIsFiring = true;
	}
}

void APlayerCharacter::StopFire()
{
	bIsFiring = false;
}

void APlayerCharacter::FireWeapon()
{
	if (dead == false)
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

		FVector TowardsLocation = BulletSpawnComp->GetComponentLocation() + (GetCapsuleComponent()->GetComponentRotation().Vector() * 5000.f);

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

		if (hitObject)
		{
			if (HitSparks != NULL)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitSparks, result.Location, FRotator::ZeroRotator, true);

			AEnemyCharacter* HitTarget = Cast<AEnemyCharacter>(result.GetActor());

			if (HitTarget)
			{
				bool enemySurvived = HitTarget->Hit(result, (TowardsLocation - BulletSpawnComp->GetComponentLocation()), 50.f);

				if (enemySurvived == false && PossessedEnemy != HitTarget)
				{
					score += (HitTarget->scoreValue / (((HitTarget->GetActorLocation() - GetActorLocation()).Size() + 10) / 100));
				}
				else if (!enemySurvived)
				{
					score += (HitTarget->scoreValue / (((HitTarget->GetActorLocation() - GetActorLocation()).Size() + 10) / 100)) / 2.f;
				}

				if (PossessedEnemy == HitTarget)
				{
					if (enemySurvived == false)
					{
						SwapRight();
						Debug::Log(FString("DÅ") );
						TVFadedTime = 0;
						TVFadeMin = 0.5;
						TVFadeMax = 0.5;
						TVFadeTime = 0.2;
						TVFadeResetSpeed = 0.15;
						TVFadeResetDelay = 0.0;
						
					}
					else
					{

						Debug::Log(FString("HEJ"));
						TVFadedTime = 0;
						TVFadeMin = 0.4;
						TVFadeMax = 0.4;
						TVFadeTime = 0.3;						
						TVFadeResetSpeed = 0.2;
						TVFadeResetDelay = 0.2;
					}
				}
			}
		}
	}
}

void APlayerCharacter::Swap(class AEnemyCharacter* Enemy)
{
	if (dead == false)
	{
		TimeInEnemy = 0.f;
		if (PossessedEnemy != NULL)
			PossessedEnemy->GetCharacterMovement()->MaxWalkSpeed = PossessedEnemy->DefaultWalkSpeed;

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
