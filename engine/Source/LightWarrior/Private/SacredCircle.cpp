#include "SacredCircle.h"

ASacredCircle::ASacredCircle()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
}

void ASacredCircle::BeginPlay()
{
    Super::BeginPlay();
    SetRadius(StartingRadius);
}

void ASacredCircle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (PassiveDarknessPressurePerSecond > 0.0f)
    {
        ApplyDarknessPressure(PassiveDarknessPressurePerSecond * DeltaSeconds);
    }
}

bool ASacredCircle::IsInsideCircle(const FVector& WorldLocation) const
{
    const FVector Delta = WorldLocation - GetActorLocation();
    const FVector FlatDelta(Delta.X, Delta.Y, 0.0f);
    return FlatDelta.SizeSquared() <= FMath::Square(CurrentRadius);
}

void ASacredCircle::ExpandFromCombat(float Amount)
{
    SetRadius(CurrentRadius + FMath::Max(0.0f, Amount));
}

void ASacredCircle::ApplyDarknessPressure(float Amount)
{
    SetRadius(CurrentRadius - FMath::Max(0.0f, Amount));
}

void ASacredCircle::SetRadius(float NewRadius)
{
    const float ClampedRadius = FMath::Clamp(NewRadius, MinRadius, MaxRadius);
    if (FMath::IsNearlyEqual(CurrentRadius, ClampedRadius))
    {
        return;
    }

    CurrentRadius = ClampedRadius;
    OnRadiusChanged.Broadcast(CurrentRadius);
}
