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

	PossessedTurnRate = 2.f;
	TurnRate = 5.f;
	Health = 100.f;
	scoreValue = 500.f;

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

	if (gameRunning)
	{
		Super::Tick(DeltaTime);

		if (isAlive)
		{
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

bool AEnemyCharacter::Hit(FHitResult HitResult, FVector FromAnge, float Damage)
{
	Health -= Damage;

	if (Health <= 0)
	{
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

		FromAnge.Normalize();
		GetMesh()->AddImpulseAtLocation(FromAnge * 10000.0f, HitResult.Location);

		DefaultGameMode->RemoveEnemy(this);
	}
	
	return isAlive;
}