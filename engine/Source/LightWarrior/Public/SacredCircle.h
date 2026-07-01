#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SacredCircle.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSacredCircleRadiusChanged, float, NewRadius);

UCLASS()
class LIGHTWARRIOR_API ASacredCircle : public AActor
{
    GENERATED_BODY()

public:
    ASacredCircle();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Sacred Circle")
    bool IsInsideCircle(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Sacred Circle")
    void ExpandFromCombat(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Sacred Circle")
    void ApplyDarknessPressure(float Amount);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Sacred Circle")
    float GetCurrentRadius() const { return CurrentRadius; }

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Sacred Circle")
    FSacredCircleRadiusChanged OnRadiusChanged;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> CircleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> MainRingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> InnerRingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> OuterInnerRingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UStaticMeshComponent> ExpansionPulseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<class UStaticMeshComponent>> BoundarySegments;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<class UStaticMeshComponent>> SpokeSegments;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UPointLightComponent> CircleLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Circle")
    float StartingRadius = 1400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Circle")
    float MinRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Circle")
    float MaxRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Circle")
    float PassiveDarknessPressurePerSecond = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sacred Circle")
    float CurrentRadius = 1400.0f;

private:
    void RefreshVisuals();
    void SetRadius(float NewRadius);
    void UpdateExpansionPulse(float DeltaSeconds);

    float ExpansionPulseTimer = 0.0f;
    float ExpansionPulseDuration = 0.82f;
    float PreviousRadius = 1400.0f;
    bool bUsingGeneratedCircleMeshes = false;
};
