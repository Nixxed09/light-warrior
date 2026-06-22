#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LightWarriorCharacter.generated.h"

class ASacredCircle;
class UCameraComponent;
class ULightWarriorProgressionComponent;
class UPointLightComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class LIGHTWARRIOR_API ALightWarriorCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ALightWarriorCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void FellOutOfWorld(const UDamageType& DamageType) override;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Combat")
    void GrantThunderHammer(float DurationSeconds);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Combat")
    bool HasThunderHammer() const { return ThunderHammerTimer > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|State")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|State")
    float GetHealthPercent() const { return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|State")
    bool IsDefeated() const { return Health <= 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Combat")
    float GetThunderHammerRemaining() const { return ThunderHammerTimer; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Progression")
    ULightWarriorProgressionComponent* GetProgressionComponent() const { return ProgressionComponent; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroHeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroLeftEyeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroRightEyeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroLeftArmMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroRightArmMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroHammerHandleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroHammerHeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeroAuraMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> HeroLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> HeroLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> StrikePulseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> StrikeLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> StrikeLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ULightWarriorProgressionComponent> ProgressionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashStrength = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashCooldown = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashDuration = 0.16f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashFriction = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float NormalGroundFriction = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float NormalBrakingDeceleration = 2600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Combat")
    float StrikeRadius = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Combat")
    float StrikeExpansionAmount = 95.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Combat")
    float OutsideCircleDamagePerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|State")
    float MaxHealth = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Light Warrior|State")
    float Health = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Light Warrior|State")
    float ThunderHammerTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|State")
    float FallDeathZ = -1200.0f;

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartDash();
    void FinishDash();
    void LightStrike();
    void CheckFallDeath();
    void KillFromFall();
    void ApplyCirclePressure(float DeltaSeconds);
    void UpdateStrikePulse(float DeltaSeconds);
    ASacredCircle* FindSacredCircle() const;

    float DashCooldownRemaining = 0.0f;
    float DashTimeRemaining = 0.0f;
    float StrikePulseTimer = 0.0f;
    float StrikePulseDuration = 0.42f;
    bool bWasInsideCircle = true;
    bool bAwardedCurrentExcursion = false;
    bool bKilledByFall = false;
};
