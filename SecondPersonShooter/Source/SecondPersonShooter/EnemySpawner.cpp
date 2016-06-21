#include "SecondPersonShooter.h"
#include "EnemySpawner.h"
#include "DefaultGameMode.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnMesh"));
	RootComponent = SpawnMesh;

	MaxEnemies = 5;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckIfToSpawn();
}

void AEnemySpawner::CheckIfToSpawn()
{
	ADefaultGameMode* GameMode;
	GameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	if (EnemyType1 != NULL && GameMode->GetNumberOfEnemies() < MaxEnemies)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			FVector SpawnLocation = RootComponent->GetComponentLocation();
			FRotator SpawnRotation = { 0, 0, 0 };

			AEnemyCharacter* SpawnedEnemy = World->SpawnActor<AEnemyCharacter>(EnemyType1, SpawnLocation, SpawnRotation, SpawnParams);
			SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed += (rand() % 100);
			SpawnedEnemy->DefaultWalkSpeed = SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed;
		}
	}
}

void AEnemySpawner::SpawnEnemy(EEnemyType EnemyType)
{
	ADefaultGameMode* GameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	if (EnemyType1 != NULL && EnemyType2 != NULL && EnemyType3 != NULL)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;

		FVector SpawnLocation = RootComponent->GetComponentLocation();
		FRotator SpawnRotation = { 0, 0, 0 };

		if (GameMode->GetPlayerRef() != NULL)
		{
			SpawnRotation = (GameMode->GetPlayerRef()->GetActorLocation() - GetActorLocation()).Rotation();
			SpawnRotation.Pitch = 0.f;
		}

		TSubclassOf<class AEnemyCharacter> Enemy;

		switch (EnemyType)
		{
		case EEnemyType::Type1:
			Enemy = EnemyType1;
			break;
		case EEnemyType::Type2:
			Enemy = EnemyType2;
			break;
		case EEnemyType::Type3:
			Enemy = EnemyType3;
			break;
		default:
			break;
		}

		AEnemyCharacter* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(Enemy, SpawnLocation, SpawnRotation, SpawnParams);
		SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed += (rand() % 100);
		SpawnedEnemy->DefaultWalkSpeed = SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed;
	}
}