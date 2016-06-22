#include "SecondPersonShooter.h"
#include "EnemyCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "AIController.h"

AEnemyCharacter::AEnemyCharacter()
{
	isAlive = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);

	PossessedTurnRate = 2.f;
	TurnRate = 5.f;
	Health = 100.f;
	scoreValue = 500.f;
	SpeedUpRate = 5.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera2"));
	Camera->AttachTo(GetMesh());
	Camera->bUsePawnControlRotation = false;
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OldRotation = GetActorRotation();
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	NavSystem = GetWorld()->GetNavigationSystem();
	AIController = Cast<AAIController>(GetController());

	DefaultGameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
	if (DefaultGameMode == NULL)
		UE_LOG(LogTemp, Fatal, TEXT("NO DEFAULT GAME MODE FOUND!"));

	DefaultGameMode->AddEnemy(this);

	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	bool gameRunning = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode())->IsGameplayRunning();

	Super::Tick(DeltaTime);

	if (gameRunning)
	{
		if (isAlive)
		{
			GetCharacterMovement()->MaxWalkSpeed += SpeedUpRate * DeltaTime;

			if (PlayerRef == NULL)
				PlayerRef = DefaultGameMode->GetPlayerRef();

			else
			{
				NavSystem->SimpleMoveToLocation(GetController(), PlayerRef->GetActorLocation());

				if (PlayerRef->GetPossessedEnemy() == this)
					OldRotation = FMath::Lerp(OldRotation, GetActorRotation(), PossessedTurnRate * DeltaTime);
				else
					OldRotation = FMath::Lerp(OldRotation, GetActorRotation(), TurnRate * DeltaTime);

				FaceRotation(OldRotation);
			}
		}
	}
}

void AEnemyCharacter::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (APlayerCharacter *player = Cast<APlayerCharacter>(OtherActor))
	{
		TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
		FDamageEvent DamageEvent(ValidDamageTypeClass);
		player->TakeDamage(40.f, DamageEvent, GetController(), this);
	}
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= DamageAmount;

	if (Health <= 0)
	{
		if (SPS::GetGameMode(this)->GetCurrentGameMode() == EGameMode::WaveMode)
		{
			if (SPS::GetGameMode(this)->GetEnemiesToSpawnInWave() == 0 && SPS::GetGameMode(this)->GetNumberOfEnemies() == 1)
			{
				Debug::LogOnScreen("I AM THE ONE AND ONLY!");
			}
		}
		isAlive = false;

		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);

		GetCapsuleComponent()->SetSimulatePhysics(false);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		GetCapsuleComponent()->SetCollisionObjectType(ECC_WorldDynamic);

		FVector FromAngle = GetActorLocation() - DamageCauser->GetActorLocation();
		FromAngle.Normalize();
		GetMesh()->AddImpulse(FromAngle * 10000.0f);

		SPS::GetGameMode(this)->RemoveEnemy(this);
	}

	return Health;
}