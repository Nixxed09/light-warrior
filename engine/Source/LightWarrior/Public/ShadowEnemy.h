#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShadowEnemy.generated.h"

class ALightWarriorCharacter;

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
    float ContactDamage = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow")
    float AttackCooldown = 2.2f;

    UPROPERTY(BlueprintReadOnly, Category = "Shadow")
    float Health = 70.0f;

private:
    ALightWarriorCharacter* FindPlayer() const;

    float AttackCooldownRemaining = 0.0f;
};
