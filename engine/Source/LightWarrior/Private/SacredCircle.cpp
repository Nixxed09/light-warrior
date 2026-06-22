#include "SacredCircle.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"

ASacredCircle::ASacredCircle()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    CircleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CircleMesh"));
    CircleMesh->SetupAttachment(SceneRoot);
    CircleMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));
    CircleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CircleMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CircleMeshAsset.Succeeded())
    {
        CircleMesh->SetStaticMesh(CircleMeshAsset.Object);
    }

    CircleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CircleLight"));
    CircleLight->SetupAttachment(SceneRoot);
    CircleLight->SetLightColor(FLinearColor(0.45f, 0.90f, 1.0f));
    CircleLight->SetIntensity(2600.0f);
    CircleLight->SetAttenuationRadius(StartingRadius);
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
    RefreshVisuals();
    OnRadiusChanged.Broadcast(CurrentRadius);
}

void ASacredCircle::RefreshVisuals()
{
    const float DiameterScale = CurrentRadius / 50.0f;
    CircleMesh->SetRelativeScale3D(FVector(DiameterScale, DiameterScale, 0.055f));
    CircleLight->SetAttenuationRadius(CurrentRadius * 1.25f);
    CircleLight->SetIntensity(FMath::Lerp(3200.0f, 8200.0f, CurrentRadius / FMath::Max(1.0f, MaxRadius)));
}
