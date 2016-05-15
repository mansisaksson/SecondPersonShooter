#include "SecondPersonShooter.h"
#include "EnemySpawner.h"
#include "DefaultGameMode.h"
#include "EnemyCharacter.h"

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

void AEnemySpawner::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//CheckIfToSpawn();
}

void AEnemySpawner::CheckIfToSpawn()
{
	ADefaultGameMode* GameMode;
	GameMode = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());

	if (WhatToSpawn != NULL && GameMode->GetNumberOfEnemies() < MaxEnemies)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			FVector SpawnLocation = RootComponent->GetComponentLocation();
			FRotator SpawnRotation = { 0,0,0 };

			AEnemyCharacter* SpawnedEnemy = World->SpawnActor<AEnemyCharacter>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed += (rand() % 100);
			SpawnedEnemy->DefaultWalkSpeed = SpawnedEnemy->GetCharacterMovement()->MaxWalkSpeed;
		}
	}
}