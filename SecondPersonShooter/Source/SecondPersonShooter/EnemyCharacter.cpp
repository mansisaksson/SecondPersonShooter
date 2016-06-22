#include "SecondPersonShooter.h"
#include "EnemyCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "AIController.h"

AEnemyCharacter::AEnemyCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AEnemyCharacter::OnHit);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera2"));
	Camera->AttachTo(GetMesh());
	Camera->bUsePawnControlRotation = false;

	TurnRate = 5.f;
	PossessedTurnRate = 2.f;
	Health = 100.f;
	scoreValue = 500.f;
	SpeedUpRate = 5.f;
	DisableTime = 0.f;

	bIsAlive = true;
	bCanTakeDamage = true;
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
		if (bIsAlive && DisableTime <= 0.f)
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
		else if (DisableTime > 0.f)
		{
			GetController()->StopMovement();
			DisableTime -= DeltaTime;

			if (DisableTime <= 0.f)
			{
				if (!bIsAlive)
					KillEnemy(FVector::ZeroVector);
				bCanTakeDamage = true;
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
	
	if (bCanTakeDamage)
	{
		Health -= DamageAmount;

		if (Health <= 0)
		{
			if (SPS::GetGameMode(this)->GetCurrentGameMode() == EGameMode::WaveMode && SPS::GetGameMode(this)->GetIsLastInWave())
				DisableEnemy(5.f, true, true);
			else
			{
				FVector FromAngle = GetActorLocation() - DamageCauser->GetActorLocation();
				FromAngle.Normalize();
				KillEnemy(FromAngle * 10000.f);
			}
		}
	}

	return Health;
}

void AEnemyCharacter::DisableEnemy(float time, bool bBlockDamage, bool bKillOnFinish)
{
	DisableTime = time;
	bCanTakeDamage = !bBlockDamage;
	
	if (bKillOnFinish)
		bIsAlive = false;
}

void AEnemyCharacter::KillEnemy(FVector Impulse)
{
	bIsAlive = false;

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

	GetMesh()->AddImpulse(Impulse);

	SPS::GetGameMode(this)->RemoveEnemy(this);
}