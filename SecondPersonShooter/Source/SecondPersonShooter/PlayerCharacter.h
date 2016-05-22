#pragma once
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS(config=Game)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
	
	UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* BulletSpawnComp;

	UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* GunMesh;

public:
	APlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return Camera; }

	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void ClearPossessedEnemy() { PossessedEnemy = NULL; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class AEnemyCharacter* GetPossessedEnemy() { return PossessedEnemy; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetRotationFromEnemy();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool GetIsAlive() {	return !bIsDead; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool GetIsFiring() { return bIsFiring; }

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	void RestartGame();
	void ExitGame();

	void MoveForward(float Value);
	void MoveRight(float Value);

	void FaceUp(float Value);
	void FaceRight(float Value);

	void SwapRight();
	void SwapLeft();
	void SelectClosestEnemy();
	void Swap(class AEnemyCharacter* Enemy);

	void StartFire();
	void StopFire();
	void FireWeapon();

	UFUNCTION(BlueprintNativeEvent)
	void OnFire();
	void OnFire_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float FadeDarkness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TVFadeValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ShotsPerSecond;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool FadeRed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 score;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 hp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	USoundBase* StaticSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* MuzzeFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* HitSparks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* TrailParticle;

	class AEnemyCharacter* PossessedEnemy;
	class ADefaultGameMode* DefaultGameMode;
	APlayerController* PlayerController;
	FVector RelativeInputRotation;

	bool bIsFiring;
	float TimeSinceFire;

	float FadeResetSpeed;
	float FadeResetDelay;
	float FadeTime;
	float FadedTime;
	float FadeMax;
	float FadeMin;

	float TVFadeResetSpeed;
	float TVFadeResetDelay;
	float TVFadeTime;
	float TVFadedTime;
	float TVFadeMax;
	float TVFadeMin;

	bool bIsDead;
	bool bHasSwappedOnce;

	float shieldTime;

	float xTurnRate;
	float yTurnRate;
};

