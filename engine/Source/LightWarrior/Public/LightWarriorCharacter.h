#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LightWarriorCharacter.generated.h"

class ASacredCircle;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class LIGHTWARRIOR_API ALightWarriorCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ALightWarriorCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Combat")
    void GrantThunderHammer(float DurationSeconds);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Combat")
    bool HasThunderHammer() const { return ThunderHammerTimer > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|State")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|State")
    bool IsDefeated() const { return Health <= 0.0f; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashStrength = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Movement")
    float DashCooldown = 0.75f;

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

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartDash();
    void LightStrike();
    void ApplyCirclePressure(float DeltaSeconds);
    ASacredCircle* FindSacredCircle() const;

    float DashCooldownRemaining = 0.0f;
};
