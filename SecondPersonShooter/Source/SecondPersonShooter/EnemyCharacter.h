#pragma once
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS(config=Game)
class AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

public:
	AEnemyCharacter();

	virtual void PostInitializeComponents();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
	void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class UCameraComponent* GetFollowCamera() const { return Camera; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetHealth() { return Health; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 scoreValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float PossessedTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float SpeedUpRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool StartOnThis;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFuction)
	bool GetIsAlive() { return isAlive; }

	float DefaultWalkSpeed;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Health;

	bool isAlive;

	FRotator OldRotation;

	UNavigationSystem* NavSystem;
	class AAIController* AIController;
	class APlayerCharacter* PlayerRef;
	class ADefaultGameMode* DefaultGameMode;
};

