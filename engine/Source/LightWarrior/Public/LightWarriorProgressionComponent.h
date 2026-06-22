#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LightWarriorProgressionComponent.generated.h"

UENUM(BlueprintType)
enum class ELightWarriorResource : uint8
{
    Light,
    Courage,
    ResonanceShards,
    TrueMapFragments,
    ShadowDebt,
    AetherCharge
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLightWarriorResourceChanged, ELightWarriorResource, Resource, float, NewValue);

UCLASS(ClassGroup = (LightWarrior), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class LIGHTWARRIOR_API ULightWarriorProgressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightWarriorProgressionComponent();

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Progression")
    void AddResource(ELightWarriorResource Resource, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Progression")
    bool SpendResource(ELightWarriorResource Resource, float Amount);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Progression")
    float GetResource(ELightWarriorResource Resource) const;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Progression")
    void RecordBraveExcursion();

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Progression")
    void RecordRestoration(float LightAmount);

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Progression")
    void RecordPassivePressure(float DebtAmount);

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Progression")
    FLightWarriorResourceChanged OnResourceChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float Light = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float Courage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float ResonanceShards = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float TrueMapFragments = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float ShadowDebt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Progression")
    float AetherCharge = 0.0f;

private:
    float* ResolveResource(ELightWarriorResource Resource);
    const float* ResolveResource(ELightWarriorResource Resource) const;
    void BroadcastResource(ELightWarriorResource Resource);
};
