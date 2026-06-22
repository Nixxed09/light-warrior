#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThunderHammerTemple.generated.h"

class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class LIGHTWARRIOR_API AThunderHammerTemple : public AActor
{
    GENERATED_BODY()

public:
    AThunderHammerTemple();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> TempleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> ActivationSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> TempleLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> TempleLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temple")
    float HammerDuration = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temple")
    float ActivationExpansionAmount = 400.0f;

private:
    UFUNCTION()
    void OnTempleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    bool bActivated = false;
};
