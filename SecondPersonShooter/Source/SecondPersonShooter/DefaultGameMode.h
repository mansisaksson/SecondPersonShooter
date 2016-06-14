#pragma once
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	MenuMode		UMETA(DisplayName = "MenuMode"),
	HordeMode		UMETA(DisplayName = "HordeMode"),
	WaveMode		UMETA(DisplayName = "WaveMode")
};

UCLASS(minimalapi)
class ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()

	typedef TDoubleLinkedList<class AEnemyCharacter*>::TDoubleLinkedListNode TNode;

public:
	ADefaultGameMode();

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

	class APlayerCharacter* GetPlayerRef();
	class AEnemyCharacter* GetClosestEnemy();
	class AEnemyCharacter* GetCloserEnemy(class AEnemyCharacter* Enemy);
	class AEnemyCharacter* GetFurtherEnemy(class AEnemyCharacter* Enemy);
	class AEnemyCharacter* GetRandomEnemy();
	class AEnemyCharacter* GetNextEnemy();
	class AEnemyCharacter* GetPrevEnemy();
	void RemoveEnemy(class AEnemyCharacter* enemy);
	void AddEnemy(class AEnemyCharacter* enemy);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool IsGameplayRunning() { return GameplayRunning; }

	UFUNCTION(BlueprintCallable, Category = GetFunction)
	void StartGameplay() { GameplayRunning = true; }

	UFUNCTION(BlueprintCallable, Category = GetFunction)
	void StartHordeMode();

	int GetNumberOfEnemies();

private:
	void UpdateMenuMode(float DeltaTime);
	void UpdateHordeMode(float DeltaTime);
	void UpdateWaveMode(float DeltaTime);

	TArray<class AEnemySpawner*> Spawners;
	TArray<class AEnemyCharacter*> Enemies;
	class APlayerCharacter* PlayerRef;

	EGameMode CurrentGameMode;

	int currentEnemyIndex;

	float timeSinceLastSpawn;
	float TotalGameTime;
	float badTimeTime;
	float spawnTime;

	bool GameplayRunning;
};



