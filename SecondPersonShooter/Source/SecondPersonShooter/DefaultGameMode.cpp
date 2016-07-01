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
	SpawnTime = 2.f;
	MaxEnemies = 3.f;

	GameplayRunning = false;
	GameLoaded = false;
	bForceStartWave = false;
	CurrentGameMode = EGameMode::MenuMode;
}

void ADefaultGameMode::BeginPlay()
{
	CurrentGameMode = EGameMode::MenuMode;

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		APlayerCharacter* player = Cast<APlayerCharacter>(*ActorItr);
		if (player != NULL)
			PlayerRef = player;
	}
}

void ADefaultGameMode::StartHordeMode()
{
	CurrentGameMode = EGameMode::HordeMode;
	SetIsLoaded(true);

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
void ADefaultGameMode::StartWaveMode()
{
	CurrentGameMode = EGameMode::WaveMode;
	SetIsLoaded(true);
	bForceStartWave = true;

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
	Super::Tick(DeltaTime);

	switch (CurrentGameMode)
	{
	case EGameMode::MenuMode:
		UpdateMenuMode(DeltaTime);
		break;
	case EGameMode::HordeMode:
		UpdateHordeMode(DeltaTime);
		break;
	case EGameMode::WaveMode:
		UpdateWaveMode(DeltaTime);
		break;
	default:
		break;
	}
	
}

void ADefaultGameMode::UpdateMenuMode(float DeltaTime)
{

}
void ADefaultGameMode::UpdateHordeMode(float DeltaTime)
{
	if (IsGameplayRunning())
	{
		TotalGameTime += DeltaTime;
		TotalBadTime += DeltaTime;
		if (TotalBadTime > badTimeTime)
		{
			TotalBadTime = 0.f;
			MaxEnemies++;
		}

		TimeSinceLastSpawn += DeltaTime;
		if (TimeSinceLastSpawn > SpawnTime)
		{
			TimeSinceLastSpawn = 0.f;

			if (GetNumberOfEnemies() < MaxEnemies)
			{
				float difficultyV = FMath::FRand();

				// super linear 50% at 2½ min, 100% at 5 min
				float shieldTreshold = 1-TotalGameTime / 300;
				if (shieldTreshold > 0.8)
					shieldTreshold = 0.8;

				//type3
				/*if(difficultyV >= type3treshhold)
				{
					Spawners[rand() % Spawners.Num()]->SpawnEnemy(EEnemyType::Type3);
				}*/
				//shield
				if (difficultyV >= shieldTreshold)
				{
					Spawners[rand() % Spawners.Num()]->SpawnEnemy(EEnemyType::Type2);
				}
				//default
				else
				{
					Spawners[rand() % Spawners.Num()]->SpawnEnemy(EEnemyType::Type1);
				}
			}
		}
	}
}
void ADefaultGameMode::UpdateWaveMode(float DeltaTime)
{
	if (IsGameplayRunning())
	{
		if ((EnemyWaves.Num() > 0 && EnemiesToSpawn.Num() == 0 && GetNumberOfEnemies() == 0) || bForceStartWave)
		{
			for (int32 i = 0; i < EnemyWaves[0].EnemyType1; i++)
				EnemiesToSpawn.Add(EEnemyType::Type1);
			for (int32 i = 0; i < EnemyWaves[0].EnemyType2; i++)
				EnemiesToSpawn.Add(EEnemyType::Type2);
			for (int32 i = 0; i < EnemyWaves[0].EnemyType3; i++)
				EnemiesToSpawn.Add(EEnemyType::Type3);

			SpawnTime = EnemyWaves[0].SpawnSpeed;
			EnemyWaves.RemoveAt(0);
			bForceStartWave = false;
		}

		if (EnemiesToSpawn.Num() > 0)
		{
			if (GetNumberOfEnemies() == 0)
			{
				int index = FMath::RandRange(0, EnemiesToSpawn.Num() - 1);
				Spawners[rand() % Spawners.Num()]->SpawnEnemy(EnemiesToSpawn[index]);
				EnemiesToSpawn.RemoveAt(index);
			}
			else if (TimeSinceLastSpawn > SpawnTime)
			{
				TimeSinceLastSpawn = 0.f;
				int index = FMath::RandRange(0, EnemiesToSpawn.Num() - 1);
				Spawners[rand() % Spawners.Num()]->SpawnEnemy(EnemiesToSpawn[index]);
				EnemiesToSpawn.RemoveAt(index);
			}
			TimeSinceLastSpawn += DeltaTime;
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
