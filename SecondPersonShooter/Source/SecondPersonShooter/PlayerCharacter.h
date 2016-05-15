#pragma once
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS(config=Game)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = Player, meta = (AllowPrivateAccesss = "true"))
	class USceneComponent* BulletSpawnComp;

	UPROPERTY(VisibleAnywhere, Category = Player, meta = (AllowPrivateAccesss = "true"))
	class UStaticMeshComponent* GunMesh;

public:
	APlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GetFunction)
	float FadeDarkness;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GetFunction)
	bool FadeRed;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GetFunction)
	float TVFadeValue;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = GetFunction)
	int32 score;

	UFUNCTION()
	void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return Camera; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class AEnemyCharacter* GetPossessedEnemy() { return PossessedEnemy; }

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
	void Swap(class AEnemyCharacter* Enemy);

	void StartFire();
	void StopFire();
	void FireWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* MuzzeFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* HitSparks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ShotsPerSecond;

	class AEnemyCharacter* PossessedEnemy;
	class ADefaultGameMode* DefaultGameMode;
	APlayerController* PlayerController;

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

	bool dead;
	int hp;
	float shieldTime;

	float xTurnRate;
	float yTurnRate;

	float TimeInEnemy;
};

