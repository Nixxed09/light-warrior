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
    void SetRadius(float NewRadius);
};
