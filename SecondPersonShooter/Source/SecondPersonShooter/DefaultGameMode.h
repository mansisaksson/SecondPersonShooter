#pragma once
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Type1		UMETA(DisplayName = "Type1"),
	Type2		UMETA(DisplayName = "Type2"),
	Type3		UMETA(DisplayName = "Type3")
};

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	MenuMode		UMETA(DisplayName = "MenuMode"),
	HordeMode		UMETA(DisplayName = "HordeMode"),
	WaveMode		UMETA(DisplayName = "WaveMode")
};

USTRUCT(BlueprintType)
struct FEnemyWave
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemyType1 = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemyType2 = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemyType3 = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnSpeed = 1.f;
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
	class AEnemyCharacter* GetCloserEnemy(class AEnemyCharacter* Enemy);
	class AEnemyCharacter* GetFurtherEnemy(class AEnemyCharacter* Enemy);
	class AEnemyCharacter* GetRandomEnemy();
	class AEnemyCharacter* GetNextEnemy();
	class AEnemyCharacter* GetPrevEnemy();
	void RemoveEnemy(class AEnemyCharacter* enemy);
	void AddEnemy(class AEnemyCharacter* enemy);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool IsGameplayRunning() { return GameplayRunning; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool IsGameLoaded() { return GameLoaded; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	int32 GetNumberOfEnemies();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	EGameMode GetCurrentGameMode() { return CurrentGameMode; };
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	int32 GetEnemiesToSpawnInWave() { return EnemiesToSpawn.Num(); };

	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SetIsLoaded(bool isLoaded) { GameLoaded = isLoaded; }
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void StartGameplay() { GameplayRunning = true; }
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void StartHordeMode();
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void StartWaveMode();
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SetEnemyWaves(TArray<FEnemyWave> EnemyWaves) { this->EnemyWaves = EnemyWaves;	};

protected:
	void UpdateMenuMode(float DeltaTime);
	void UpdateHordeMode(float DeltaTime);
	void UpdateWaveMode(float DeltaTime);

	TArray<class AEnemySpawner*> Spawners;
	TArray<class AEnemyCharacter*> Enemies;
	class APlayerCharacter* PlayerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	EGameMode CurrentGameMode;

	// Wave Mode Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WaveMode)
	TArray<FEnemyWave> EnemyWaves;
	TArray<EEnemyType> EnemiesToSpawn;

	bool bForceStartWave;

	// Horde Mode Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HordeMode)
	float InitialSpawnRate;

	float TotalGameTime;
	float badTimeTime;


	// Shared Variables
	int currentEnemyIndex;

	float TimeSinceWaveFinish;
	float TimeSinceLastSpawn;
	float SpawnTime;

	bool GameplayRunning;
	bool GameLoaded;
};



