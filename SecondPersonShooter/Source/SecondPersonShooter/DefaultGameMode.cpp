#include "SecondPersonShooter.h"
#include "DefaultGameMode.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "EnemySpawner.h"
#include "Online.h"
#include "Engine.h"

#include "../../Plugins/OculusPlatformPlugin/Source/ThirdParty/Oculus/LibOVRPlatform/LibOVRPlatform/include/OVR_Platform.h"

static int ConnectAttempts;
static int MaxConnectAttempts;
static bool bIsAuthorized;

ADefaultGameMode::ADefaultGameMode()
{
	currentEnemyIndex = -1;
	badTimeTime = 10.f;
	SpawnTime = 2.f;
	MaxEnemies = 3.f;
	ConnectAttempts = 0.f;
	MaxConnectAttempts = 10.f;

	GameplayRunning = false;
	GameLoaded = false;
	bForceStartWave = false;
	bIsAuthorized = false;
	bGetScoreFromServer = false;
	bSendScoreToServer = false;
	bHasUpdatedScore = false;
	CurrentGameMode = EGameMode::MenuMode;

	TimeToResendMessage = 0;
	TimeToTimeOutMessage = 15;
}

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();
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

void ADefaultGameMode::OnPlayerDeath()
{
	bGetScoreFromServer = true;
	TimeToTimeOutMessage = 15.f;
}

void ADefaultGameMode::AuthorizeUser()
{
	UDebug::LogOnScreen("Authorization successful", 20.f);
	ConnectAttempts = MaxConnectAttempts;
	bIsAuthorized = true;
}

void ADefaultGameMode::Shutdown()
{
	UDebug::LogOnScreen("Authorization failed, shutting down... not really ;)", 20.f);
	bIsAuthorized = false;
}

void ADefaultGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAuthorized)
	{
		TimeToResendMessage -= DeltaTime;
		TimeToTimeOutMessage -= DeltaTime;
		if (TimeToTimeOutMessage > 0)
		{
			if (bGetScoreFromServer && TimeToResendMessage <= 0)
			{
				TimeToResendMessage = 0.2f;
				UDebug::LogOnScreen("Sending Leader Board Get Request.");
				ovr_Leaderboard_GetEntries("warehouse1", 10, ovrLeaderboard_FilterNone, ovrLeaderboard_StartAtTop);
			}
			else if (bSendScoreToServer && TimeToResendMessage <= 0)
			{
				TimeToResendMessage = 0.2;
				UDebug::LogOnScreen("Sending Leader Board Write Request.");
				// For Write Request: 
				//ovr_Leaderboard_WriteEntry ("LEADERBOARD-NAME", [USER-SCORE], [OPTIONAL GAME DATA], [SIZE OF OPTIONAL DATA], false)
			}
		}

		ovrMessage *response = ovr_PopMessage();
		if (response)
		{
			int messageType = ovr_Message_GetType(response);

			if (messageType == ovrMessage_Entitlement_GetIsViewerEntitled)
			{
				UDebug::LogOnScreen("Entitlement request Received.", 10.f, FColor::Yellow);
			}
			else if (messageType == ovrMessage_Leaderboard_GetEntries)
			{
				bGetScoreFromServer = false;
				TimeToTimeOutMessage = 15.f;

				if (ovr_Message_IsError(response) != 0) 
				{
					FString stringuu(ovr_Error_GetMessage(ovr_Message_GetError(response)));
					UDebug::LogOnScreen("Error: " + stringuu, 20.f, FColor::Red);
				}
				else
				{
					UDebug::LogOnScreen("Received Leader Board Get Message", 10.f, FColor::Green);

					// Retrieve your ovrLeaderboardEntryArrayHandle
					ovrLeaderboardEntryArrayHandle leaderboards = ovr_Message_GetLeaderboardEntryArray(response);
					int count = ovr_LeaderboardEntryArray_GetSize(leaderboards);
					if (count > 0) {
						ovrLeaderboardEntryHandle firstItem = ovr_LeaderboardEntryArray_GetElement(leaderboards, 0);
					}

					// Gör iårdning något att skicka till servern
					// Cool kod :D
					// Suck code much wow

					bSendScoreToServer = true;
				}
			}
			if (messageType == ovrMessage_Leaderboard_WriteEntry)
			{
				bSendScoreToServer = false;
				TimeToTimeOutMessage = 15.f;

				if (ovr_Message_IsError(response) != 0) 
				{
					FString stringuu(ovr_Error_GetMessage(ovr_Message_GetError(response)));
					UDebug::LogOnScreen("Error: " + stringuu, 20.f, FColor::Red);
				}
				else
				{
					UDebug::LogOnScreen("Received Leader Board Write Message.", 10.f, FColor::Green);

					// Retrieve your ovrLeaderboardUpdateStatusHandle
					ovrLeaderboardUpdateStatusHandle updateStatus = ovr_Message_GetLeaderboardUpdateStatus(response);
					bool didLeaderboardUpdate = ovr_LeaderboardUpdateStatus_GetDidUpdate(updateStatus);

					if (didLeaderboardUpdate)
						UDebug::LogOnScreen("Leader Board Updated Successfully.", FColor::Green);
					else
						UDebug::LogOnScreen("Failed to Update Leader Board.", FColor::Red);
				}
			}
			else {
				UDebug::LogOnScreen("Unknown OVR Request Received.", 10.f, FColor::Yellow);
			}
			ovr_FreeMessage(response);
		}
	}

	if (ConnectAttempts < MaxConnectAttempts)
	{
		UDebug::LogOnScreen("Attempting user validation...", 15.f);
		if (Online::GetIdentityInterface().IsValid())
		{
			if (Online::GetIdentityInterface()->GetUniquePlayerId(0).IsValid())
			{
				Online::GetIdentityInterface()->GetUserPrivilege(
					*Online::GetIdentityInterface()->GetUniquePlayerId(0),
					EUserPrivileges::CanPlay,
					IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateLambda([](const FUniqueNetId &UserId, EUserPrivileges::Type Privilege, uint32 CheckResult)
					{
						if (CheckResult != (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
							ADefaultGameMode::Shutdown();

						else
							ADefaultGameMode::AuthorizeUser();
					}));
			}
			else
			{
				UDebug::LogOnScreen("Failed to get Unique Player ID", 15.f);
			}
		}
		else
		{
			UDebug::LogOnScreen("Failed to get Identity Interface", 15.f);
		}
		ConnectAttempts++;
	}

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

bool ADefaultGameMode::GetIsAuthorized()
{
	return bIsAuthorized;
}

int ADefaultGameMode::GetNumberOfEnemies()
{
	return Enemies.Num();
}
