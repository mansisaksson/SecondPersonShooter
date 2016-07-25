#pragma once
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	StarterWeapon		UMETA(DisplayName = "StarterWeapon"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	Laser		UMETA(DisplayName = "Laser")
};


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
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void ClearPossessedEnemy() { PossessedEnemy = NULL; }
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void AddScore(float s) { score += s; }

	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SwapCloser();
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SwapFurther();
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void SwapRandom();
	UFUNCTION(BlueprintCallable, Category = SetFunction)
	void PossessEnemy(class AEnemyCharacter* Enemy);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class UCameraComponent* GetFollowCamera() const { return Camera; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	class AEnemyCharacter* GetPossessedEnemy() { return PossessedEnemy; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetRotationFromEnemy();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool GetIsAlive() {	return !bIsDead; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	bool GetIsFiring() { return bIsFiring; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetHealth() { return Health; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetMaxHealth() { return MaxHealth; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetSpecial() { return Special; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = GetFunction)
	float GetMaxSpecial() { return MaxSpecial; }

	UFUNCTION(BlueprintCallable, Category = CallFunction)
	void PossessedIsKilled();
	UFUNCTION(BlueprintCallable, Category = CallFunction)
	void PossessedIsDamaged();
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	void RestartGame();
	void ExitGame();

	void MoveForward(float Value);
	void MoveRight(float Value);


	
	void FaceUp(float Value);
	void FaceRight(float Value);
	void SwapCloser_Input();
	void SwapFurther_Input();
	void SwapRandom_Input();
	void StartFire();
	void StopFire();
	void FireWeapon();
	void FireNormalWeapon();
	void FireLaserWeapon();
	void UpdatePowerups(float DeltaSeconds);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnFire();
	void OnFire_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TurnRate;

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
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Special;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxSpecial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float SuperSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float AttackSpeedBonus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float SuperWeaponSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MoveSpeedBonus;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* LaserParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* LaserHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visual)
	UParticleSystem* ShieldHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	EWeaponType weapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float SuperWeapon;
	
	class AEnemyCharacter* PossessedEnemy;
	class AEnemyCharacter* HighlightedEnemy;
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

	bool bIsDead;

	float shieldTime;

	FVector RelativeInputRotation;
	float xTurnRate;
	float yTurnRate;
	float xMoveDirection;
	float yMoveDirection;
};

