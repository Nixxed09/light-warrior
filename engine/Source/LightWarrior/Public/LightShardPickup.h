#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightShardPickup.generated.h"

class UPointLightComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;
class ALightWarriorCharacter;

UCLASS()
class LIGHTWARRIOR_API ALightShardPickup : public AActor
{
    GENERATED_BODY()

public:
    ALightShardPickup();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ShardMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PickupSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> ShardLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Pickup")
    float LightReward = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Pickup")
    float CourageReward = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Pickup")
    float ResonanceShardReward = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Pickup")
    float CircleExpansionReward = 24.0f;

private:
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void Collect(ALightWarriorCharacter* Character);

    float LifeTimeSeconds = 0.0f;
    bool bCollected = false;
};
