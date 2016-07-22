#pragma once
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS(config=Game)
class AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* CameraPosition;

public:
	AEnemyCharacter();

	virtual void PostInitializeComponents();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void DisableEnemy(float time, bool bBlockDamage = false, bool bKillOnFinish = false);
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void KillEnemy(FVector Impulse);
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SetDefaultWalkSpeed(float Speed) { DefaultWalkSpeed = Speed; };
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = SetFunction)
	void SetEnemyHighlighted(bool Highlight); // Gör mer skit i Blueprint

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class UCameraComponent* GetCamera() const { return Camera; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetHealth() { return Health; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFuction)
	bool GetIsAlive() { return bIsAlive; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFuction)
	bool GetIsDisabled() { return DisableTime > 0.f; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFuction)
	int32 GetScoreValue() { return scoreValue; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFuction)
	FRotator GetActualRotation() { return OldRotation; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool StartOnThis;

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeath"))
	void OnDeath();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 scoreValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 scoreGivenOnDeath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float PossessedTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float SpeedUpRate;
	

	float DefaultWalkSpeed;
	float DisableTime;

	bool bIsAlive;
	bool bCanTakeDamage;
	bool bKillOnFinish;

	FRotator OldRotation;

	UNavigationSystem* NavSystem;
	class AAIController* AIController;
	class APlayerCharacter* PlayerRef;
	class ADefaultGameMode* DefaultGameMode;
};

