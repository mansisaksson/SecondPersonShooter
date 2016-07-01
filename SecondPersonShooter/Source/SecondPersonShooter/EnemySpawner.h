#pragma once
#include "GameFramework/Actor.h"
#include "DefaultGameMode.h"
#include "EnemySpawner.generated.h"

UCLASS()
class SECONDPERSONSHOOTER_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:	
	AEnemySpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void CheckIfToSpawn();
	void SpawnEnemy(EEnemyType EnemyType);

	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SpawnMesh;

protected:
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TSubclassOf<class AEnemyCharacter> EnemyType1;
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TSubclassOf<class AEnemyCharacter> EnemyType2;
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TSubclassOf<class AEnemyCharacter> EnemyType3;

};
