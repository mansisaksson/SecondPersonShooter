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
ovrUserHandle PlayerHandle;

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
	bGetTopScoreFromServer = false;

	CurrentGameMode = EGameMode::MenuMode;

	TimeToResendMessage = 0;
	TimeOutTime = 30.f;
	TimeToTimeOutMessage = TimeOutTime;
}

void ADefaultGameMode::BeginPlay()
{
	TimeToTimeOutMessage = TimeOutTime;

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
	bSendScoreToServer = true;
	TimeToTimeOutMessage = TimeOutTime;
}

void ADefaultGameMode::AuthorizeUser()
{
	UDebug::LogOnScreen("Authorization successful", 20.f);
	ConnectAttempts = MaxConnectAttempts;
	bIsAuthorized = true;
	ovr_User_GetLoggedInUser();
	//Online::GetIdentityInterface()->GetUserAccount(*Online::GetIdentityInterface()->GetUniquePlayerId(0).Get())->GetDisplayName();
}

void ADefaultGameMode::Shutdown()
{
	UDebug::LogOnScreen("Authorization failed, shutting down... not really ;)", 20.f);
	bIsAuthorized = false;
}

void ADefaultGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	if (bIsAuthorized)
	{
		TimeToResendMessage -= DeltaTime;
		TimeToTimeOutMessage -= DeltaTime;
		if (TimeToTimeOutMessage > 0)
		{
			if (bGetScoreFromServer && TimeToResendMessage <= 0)
			{
				TimeToResendMessage = 0.1f;
				UDebug::LogOnScreen("Sending Leaderboard Get Request.");
				ovr_Leaderboard_GetEntries(TCHAR_TO_ANSI(*CurrentLevel.ToLower()), 3, ovrLeaderboard_FilterNone, ovrLeaderboard_StartAtCenteredOnViewer);
			}
			else if (bGetTopScoreFromServer && TimeToResendMessage <= 0)
			{
				TimeToResendMessage = 0.1f;
				UDebug::LogOnScreen("Sending Leaderboard Get Request (Top User).");
				ovr_Leaderboard_GetEntries(TCHAR_TO_ANSI(*CurrentLevel.ToLower()), 1, ovrLeaderboard_FilterNone, ovrLeaderboard_StartAtTop);
			}
			else if (bSendScoreToServer && TimeToResendMessage <= 0)
			{
				TimeToResendMessage = 0.1;
				UDebug::LogOnScreen("Sending Leaderboard Write Request.");
				long long score = PlayerRef->GetScore();
				ovr_Leaderboard_WriteEntry(TCHAR_TO_ANSI(*CurrentLevel.ToLower()), score, NULL, NULL, false);
			}
		}
		else if (bGetTopScoreFromServer || bGetScoreFromServer || bSendScoreToServer)
		{
			UDebug::LogOnScreen("Message Timed Out!", 15.f, FColor::Red);
			// Something Timed out
			bGetTopScoreFromServer = false;
			bGetScoreFromServer = false;
			bSendScoreToServer = false;
		}

		ovrMessage* response = ovr_PopMessage();
		while (response)
		{
			int messageType = ovr_Message_GetType(response);

			if (messageType == ovrMessage_User_GetLoggedInUser)
			{
				if (ovr_Message_IsError(response) != 0)
					UDebug::LogOnScreen(ovr_Error_GetMessage(ovr_Message_GetError(response)), 20.f, FColor::Red);

				else
				{
					PlayerHandle = ovr_Message_GetUser(response);
					PlayerOculusName = ovr_User_GetOculusID(PlayerHandle);
				}
			}
			else if (messageType == ovrMessage_Leaderboard_GetEntries)
			{
				if (ovr_Message_IsError(response) != 0)
					UDebug::LogOnScreen(ovr_Error_GetMessage(ovr_Message_GetError(response)), 20.f, FColor::Red);

				else if (bGetScoreFromServer)
				{
					bGetScoreFromServer = false;
					TimeToTimeOutMessage = TimeOutTime;

					UDebug::LogOnScreen("Received Leader Board Get Message", 10.f, FColor::Green);
					ovrLeaderboardEntryArrayHandle leaderboards = ovr_Message_GetLeaderboardEntryArray(response);
					size_t count = ovr_LeaderboardEntryArray_GetSize(leaderboards);

					PlayerScores.Empty();
					for (size_t i = 0; i < count; i++)
					{
						ovrLeaderboardEntryHandle Entry = ovr_LeaderboardEntryArray_GetElement(leaderboards, i);

						FPlayerScore PlayerScore;
						PlayerScore.Score = (int32)ovr_LeaderboardEntry_GetScore(Entry);
						PlayerScore.Rank = (int32)ovr_LeaderboardEntry_GetRank(Entry);
						ovrUserHandle handle = ovr_LeaderboardEntry_GetUser(Entry);
						PlayerScore.PlayerName = FString(ovr_User_GetOculusID(handle));

						PlayerScores.Add(PlayerScore);
						UDebug::LogOnScreen("User Score: " + PlayerScore.PlayerName + FString::Printf(TEXT(" - Score: %i - Rank: %i"), PlayerScore.Score, PlayerScore.Rank), 20.f, FColor::Emerald);
						//bHasUpdatedScore = true;
					}

					if (PlayerScores.Num() > 0 && PlayerScores[0].Rank != 1)
						bGetTopScoreFromServer = true;
					else
						bHasUpdatedScore = true;
				}
				else if (bGetTopScoreFromServer)
				{
					bGetTopScoreFromServer = false;
					TimeToTimeOutMessage = TimeOutTime;

					UDebug::LogOnScreen("Received Leader Board Get Message (Top User)", 10.f, FColor::Green);
					ovrLeaderboardEntryArrayHandle leaderboards = ovr_Message_GetLeaderboardEntryArray(response);
					size_t count = ovr_LeaderboardEntryArray_GetSize(leaderboards);

					for (size_t i = 0; i < count; i++)
					{
						ovrLeaderboardEntryHandle Entry = ovr_LeaderboardEntryArray_GetElement(leaderboards, i);

						FPlayerScore PlayerScore;
						PlayerScore.Score = (int32)ovr_LeaderboardEntry_GetScore(Entry);
						PlayerScore.Rank = (int32)ovr_LeaderboardEntry_GetRank(Entry);
						ovrUserHandle handle = ovr_LeaderboardEntry_GetUser(Entry);
						PlayerScore.PlayerName = FString(ovr_User_GetOculusID(handle));

						PlayerScores.Insert(PlayerScore, 0);
						UDebug::LogOnScreen("User Score: " + PlayerScore.PlayerName + FString::Printf(TEXT(" - Score: %i - Rank: %i"), PlayerScore.Score, PlayerScore.Rank), 20.f, FColor::Emerald);
						bHasUpdatedScore = true;
					}
				}
			}
			else if (messageType == ovrMessage_Leaderboard_WriteEntry)
			{
				bSendScoreToServer = false;
				TimeToTimeOutMessage = TimeOutTime;

				if (ovr_Message_IsError(response) != 0)
				{
					UDebug::LogOnScreen(ovr_Error_GetMessage(ovr_Message_GetError(response)), 20.f, FColor::Red);
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

				bGetScoreFromServer = true;
			}
			else
			{
				UDebug::LogOnScreen("Unknown OVR Request Received.", 10.f, FColor::Yellow);
				UDebug::LogOnScreen(FString::Printf(TEXT("%i"), messageType), 10.f, FColor::Yellow);
			}
			ovr_FreeMessage(response);
			response = ovr_PopMessage();
		}
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

FString ADefaultGameMode::GetOculusName()
{
	return PlayerOculusName;
}
