#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightWell.generated.h"

class ALightWarriorCharacter;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLightWellPurificationChanged, float, Purification01, bool, bIsBeingPurified);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLightWellPurificationStarted, class ALightWell*, LightWell);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLightWellPurified, class ALightWell*, LightWell);

UCLASS()
class LIGHTWARRIOR_API ALightWell : public AActor
{
    GENERATED_BODY()

public:
    ALightWell();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Light Well")
    bool IsPurified() const { return bPurified; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Light Well")
    float GetPurification01() const;

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Light Well")
    bool IsBeingPurified() const { return bPlayerInside && !bPurified; }

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Light Well")
    FLightWellPurificationChanged OnPurificationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Light Well")
    FLightWellPurificationStarted OnPurificationStarted;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Light Well")
    FLightWellPurified OnPurified;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> WellMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> RestorationRingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PurificationSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPointLightComponent> WellLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> WellLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float PurificationRadius = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float PurificationSeconds = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float DecayDelaySeconds = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float DecaySeconds = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float CircleExpansionOnPurified = 360.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float RestorationBurstRadius = 980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Light Well")
    float RestorationBurstDamage = 120.0f;

private:
    UFUNCTION()
    void OnPurificationBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPurificationEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void SetPlayerInside(bool bNewPlayerInside);
    void SetPurificationProgress(float NewProgress);
    void CompletePurification();
    void ExpandSacredCircle() const;
    void RepelNearbyShadows();

    UPROPERTY()
    TObjectPtr<ALightWarriorCharacter> PurifyingCharacter;

    float PurificationProgress = 0.0f;
    float TimeSincePlayerLeft = 0.0f;
    bool bPlayerInside = false;
    bool bHasStartedPurification = false;
    bool bPurified = false;
};
