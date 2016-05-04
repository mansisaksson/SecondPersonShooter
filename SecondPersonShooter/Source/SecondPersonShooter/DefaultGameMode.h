#pragma once
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"
UCLASS(minimalapi)
class ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ADefaultGameMode();

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

	class APlayerCharacter* GetPlayerRef();
	class AEnemyCharacter* GetClosestEnemy();
	class AEnemyCharacter* GetNextEnemy();
	class AEnemyCharacter* GetPrevEnemy();
	void RemoveEnemy(class AEnemyCharacter* enemy);
	void AddEnemy(class AEnemyCharacter* enemy);

	//TArray<class AEnemyCharacter*> GetEnemies();
	int GetNumberOfEnemies();

private:
	TArray<class AEnemySpawner*> Spawners;

	TDoubleLinkedList<class AEnemyCharacter*> Enemies;
	TDoubleLinkedList<class AEnemyCharacter*>::TDoubleLinkedListNode* EnemyNode;

	class APlayerCharacter* PlayerRef;

	int currentEnemyIndex;

	float timeSinceLastSpawn;

	float TotalGameTime;
	float badTimeTime;
	float spawnTime;
};



