#include "SecondPersonShooter.h"
#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "EnemySpawner.h"
#include "Engine.h"

ADefaultGameMode::ADefaultGameMode()
{
	currentEnemyIndex = 0;
	EnemyNode = NULL;
	badTimeTime = 10.f;
	spawnTime = 3.f;
}

void ADefaultGameMode::BeginPlay()
{
	//Enemies.Empty();
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AEnemySpawner* spawner = Cast<AEnemySpawner>(*ActorItr);
		if (spawner != NULL)
			Spawners.Add(spawner);
	}

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		APlayerCharacter* player = Cast<APlayerCharacter>(*ActorItr);
		if (player != NULL)
			PlayerRef = player;
	}
}

void ADefaultGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TotalGameTime += DeltaTime;
	if (TotalGameTime > badTimeTime)
	{
		TotalGameTime = 0.f;
		if (badTimeTime < 8.f)
			badTimeTime -= 3.f;

		for (size_t i = 0; i < Spawners.Num(); i++)
			Spawners[i]->MaxEnemies++;

		spawnTime /= 1.5f;
	}

	timeSinceLastSpawn += DeltaTime;
	if (timeSinceLastSpawn > 1.f)
	{
		timeSinceLastSpawn = 0;
		Spawners[rand() % Spawners.Num()]->CheckIfToSpawn();
	}
}
void ADefaultGameMode::AddEnemy(AEnemyCharacter* enemy)
{
	Enemies.AddTail(enemy);
}

void ADefaultGameMode::RemoveEnemy(AEnemyCharacter* enemy)
{
	if (EnemyNode != NULL && EnemyNode->GetValue() == enemy)
	{
		EnemyNode = NULL;
	}
	Enemies.RemoveNode(enemy);
}

APlayerCharacter* ADefaultGameMode::GetPlayerRef()
{
	return PlayerRef;
}


AEnemyCharacter* ADefaultGameMode::GetClosestEnemy()
{

	return NULL;
}

AEnemyCharacter* ADefaultGameMode::GetNextEnemy()
{
	
	if (Enemies.Num() > 0)
	{
		if (EnemyNode == NULL)
		{
			EnemyNode = Enemies.GetHead();
			return EnemyNode->GetValue();
		}
		else if (EnemyNode != Enemies.GetTail())
		{
			EnemyNode = EnemyNode->GetNextNode();
		}
		else
		{
			EnemyNode = Enemies.GetHead();
		}
		return EnemyNode->GetValue();
	}

	return NULL;
}

AEnemyCharacter* ADefaultGameMode::GetPrevEnemy()
{
	if (Enemies.Num() > 0)
	{
		if (EnemyNode == NULL)
		{
			EnemyNode = Enemies.GetTail();
			return EnemyNode->GetValue();
		}
		else if (EnemyNode != Enemies.GetHead())
		{
			EnemyNode = EnemyNode->GetPrevNode();
		}
		else
		{
			EnemyNode = Enemies.GetTail();
		}
		return EnemyNode->GetValue();
	}
	return NULL;
}

int ADefaultGameMode::GetNumberOfEnemies()
{
	return Enemies.Num();
}

