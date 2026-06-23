#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShadowEnemy.generated.h"

class ALightWarriorCharacter;
class UPointLightComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class LIGHTWARRIOR_API AShadowEnemy : public ACharacter
{
    GENERATED_BODY()

public:
    AShadowEnemy();

    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float MaxHealth = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float ContactDamage = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float AttackRange = 185.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float AttackCooldown = 2.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float AttackWindupSeconds = 0.62f;

    UPROPERTY(BlueprintReadOnly, Category = "Shadow")
    float Health = 70.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ShadowMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> AttackTellMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> ShadowLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> ShadowLabel;

private:
    ALightWarriorCharacter* FindPlayer() const;
    void StartAttackWindup();
    void FinishAttack(ALightWarriorCharacter* Player);
    void UpdateAttackTell(float DeltaSeconds);
    void UpdateHitFeedback(float DeltaSeconds);
    void SpawnDeathBurst() const;

    float AttackCooldownRemaining = 0.0f;
    float AttackWindupRemaining = 0.0f;
    float HitFeedbackRemaining = 0.0f;
    bool bAttackWindingUp = false;
};
