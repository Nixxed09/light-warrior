#include "SacredCircle.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LightWarriorAudio.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
void ApplyCircleColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color)
{
    if (!MeshComponent)
    {
        return;
    }

    UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/ArtTools/RenderToTexture/Materials/Debug/M_Emissive_Color.M_Emissive_Color"));
    if (!Material)
    {
        Material = MeshComponent->GetMaterial(0);
    }

    if (Material)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, MeshComponent);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
            DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}
}

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

    ExpansionPulseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExpansionPulseMesh"));
    ExpansionPulseMesh->SetupAttachment(SceneRoot);
    ExpansionPulseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -4.0f));
    ExpansionPulseMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.025f));
    ExpansionPulseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ExpansionPulseMesh->SetHiddenInGame(true);
    if (CircleMeshAsset.Succeeded())
    {
        ExpansionPulseMesh->SetStaticMesh(CircleMeshAsset.Object);
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
    ApplyCircleColor(CircleMesh, FLinearColor(0.34f, 0.92f, 1.0f));
    ApplyCircleColor(ExpansionPulseMesh, FLinearColor(1.0f, 0.86f, 0.22f));
    SetRadius(StartingRadius);
}

void ASacredCircle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (PassiveDarknessPressurePerSecond > 0.0f)
    {
        ApplyDarknessPressure(PassiveDarknessPressurePerSecond * DeltaSeconds);
    }

    UpdateExpansionPulse(DeltaSeconds);
}

bool ASacredCircle::IsInsideCircle(const FVector& WorldLocation) const
{
    const FVector Delta = WorldLocation - GetActorLocation();
    const FVector FlatDelta(Delta.X, Delta.Y, 0.0f);
    return FlatDelta.SizeSquared() <= FMath::Square(CurrentRadius);
}

void ASacredCircle::ExpandFromCombat(float Amount)
{
    if (Amount > 0.0f)
    {
        PreviousRadius = CurrentRadius;
        ExpansionPulseTimer = ExpansionPulseDuration;
        if (ExpansionPulseMesh)
        {
            ExpansionPulseMesh->SetHiddenInGame(false);
        }
        ULightWarriorAudio::PlaySfx(this, ELightWarriorSfx::CircleExpand, GetActorLocation(), 0.75f);
    }

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

void ASacredCircle::UpdateExpansionPulse(float DeltaSeconds)
{
    if (ExpansionPulseTimer <= 0.0f || !ExpansionPulseMesh)
    {
        return;
    }

    ExpansionPulseTimer = FMath::Max(0.0f, ExpansionPulseTimer - DeltaSeconds);
    const float Alpha = 1.0f - ExpansionPulseTimer / FMath::Max(0.01f, ExpansionPulseDuration);
    const float PulseRadius = FMath::Lerp(PreviousRadius, CurrentRadius + 185.0f, Alpha);
    const float DiameterScale = PulseRadius / 50.0f;
    ExpansionPulseMesh->SetRelativeScale3D(FVector(DiameterScale, DiameterScale, 0.035f));
    CircleLight->SetIntensity(FMath::Lerp(16000.0f, 8200.0f, Alpha));

    if (ExpansionPulseTimer <= 0.0f)
    {
        ExpansionPulseMesh->SetHiddenInGame(true);
        RefreshVisuals();
    }
}
