#include "SecondPersonShooter.h"
#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "EnemySpawner.h"
#include "Engine.h"

ADefaultGameMode::ADefaultGameMode()
{
	currentEnemyIndex = -1;
	badTimeTime = 10.f;
	spawnTime = 3.f;

	GameplayRunning = false;
}

void ADefaultGameMode::BeginPlay()
{
	//Enemies.Empty();
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AEnemySpawner* spawner = Cast<AEnemySpawner>(*ActorItr);
		if (spawner != NULL)
			Spawners.Add(spawner);

		APlayerCharacter* player = Cast<APlayerCharacter>(*ActorItr);
		if (player != NULL)
			PlayerRef = player;
	}
}

void ADefaultGameMode::Tick(float DeltaTime)
{
	bool gameRunning = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode())->IsGameplayRunning();

	if (gameRunning)
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
}
void ADefaultGameMode::AddEnemy(AEnemyCharacter* enemy)
{
	Enemies.Add(enemy);
}

void ADefaultGameMode::RemoveEnemy(AEnemyCharacter* enemy)
{
	if (Enemies.Num() > 0 && currentEnemyIndex < Enemies.Num() && currentEnemyIndex != -1)
	{
		if (Enemies[currentEnemyIndex] == enemy)
			currentEnemyIndex = -1;
	}
	Enemies.Remove(enemy);
}

APlayerCharacter* ADefaultGameMode::GetPlayerRef()
{
	return PlayerRef;
}

AEnemyCharacter* ADefaultGameMode::GetClosestEnemy()
{
	AEnemyCharacter* ClosestEnemy = NULL;
	if (Enemies.Num() > 0)
	{

	}

	return ClosestEnemy;
}

AEnemyCharacter* ADefaultGameMode::GetCloserEnemy(class AEnemyCharacter* Enemy)
{
	if (Enemy != NULL && PlayerRef != NULL)
	{
		AEnemyCharacter* ClosestCandidate = Enemy;
		float ReferencePlayerDist = FVector::Dist(Enemy->GetActorLocation(), PlayerRef->GetActorLocation());
		float DistToClosestCandidate = 0xfffffffff;

		for (int i = 0; i < Enemies.Num(); i++)
		{
			if (Enemies[i] != Enemy)
			{
				float PlayerDist = FVector::Dist(Enemies[i]->GetActorLocation(), PlayerRef->GetActorLocation());
				float DistToEnemy = FVector::Dist(Enemies[i]->GetActorLocation(), Enemy->GetActorLocation());

				if (PlayerDist < ReferencePlayerDist && DistToEnemy < DistToClosestCandidate)
				{
					DistToClosestCandidate = DistToEnemy;
					ClosestCandidate = Enemies[i];
				}
			}
		}

		currentEnemyIndex = Enemies.Find(ClosestCandidate, currentEnemyIndex);
		return ClosestCandidate;
	}
	return NULL;
}
AEnemyCharacter* ADefaultGameMode::GetFurtherEnemy(class AEnemyCharacter* Enemy)
{
	if (Enemy != NULL && PlayerRef != NULL)
	{
		AEnemyCharacter* ClosestCandidate = Enemy;
		float ReferencePlayerDist = FVector::Dist(Enemy->GetActorLocation(), PlayerRef->GetActorLocation());
		float DistToClosestCandidate = 0xfffffffff;// ReferencePlayerDist * 99999.f;

		for (int i = 0; i < Enemies.Num(); i++)
		{
			if (Enemies[i] != Enemy)
			{
				float PlayerDist = FVector::Dist(Enemies[i]->GetActorLocation(), PlayerRef->GetActorLocation());
				float DistToEnemy = FVector::Dist(Enemies[i]->GetActorLocation(), Enemy->GetActorLocation());

				if (PlayerDist > ReferencePlayerDist && DistToEnemy < DistToClosestCandidate)
				{
					DistToClosestCandidate = DistToEnemy;
					ClosestCandidate = Enemies[i];
				}
			}
		}

		currentEnemyIndex = Enemies.Find(ClosestCandidate, currentEnemyIndex);
		return ClosestCandidate;
	}
	return NULL;
}

AEnemyCharacter* ADefaultGameMode::GetNextEnemy()
{
	if (Enemies.Num() > 0)
	{
		if (currentEnemyIndex == -1)
			currentEnemyIndex = 0;

		else if (currentEnemyIndex < Enemies.Num()-1)
			currentEnemyIndex++;

		else
			currentEnemyIndex = 0;

		return Enemies[currentEnemyIndex];
	}

	return NULL;
}
AEnemyCharacter* ADefaultGameMode::GetPrevEnemy()
{
	if (Enemies.Num() > 0)
	{
		if (currentEnemyIndex == -1)
			currentEnemyIndex = 0;

		else if (currentEnemyIndex > 0)
			currentEnemyIndex--;

		else
			currentEnemyIndex = Enemies.Num()-1;

		return Enemies[currentEnemyIndex];
	}
	return NULL;
}
AEnemyCharacter* ADefaultGameMode::GetRandomEnemy()
{
	if (Enemies.Num() > 0)
		return Enemies[FMath::RandRange(0, Enemies.Num() - 1)];
	return NULL;
}

int ADefaultGameMode::GetNumberOfEnemies()
{
	return Enemies.Num();
}

