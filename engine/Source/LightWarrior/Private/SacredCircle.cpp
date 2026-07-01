#include "SacredCircle.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LightWarriorAudio.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
void ApplyCircleColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color, bool bEmissive)
{
    if (!MeshComponent)
    {
        return;
    }

    UMaterialInterface* Material = LoadObject<UMaterialInterface>(
        nullptr,
        bEmissive
            ? TEXT("/Engine/ArtTools/RenderToTexture/Materials/Debug/M_Emissive_Color.M_Emissive_Color")
            : TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
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
            const int32 MaterialSlots = FMath::Max(1, MeshComponent->GetNumMaterials());
            for (int32 SlotIndex = 0; SlotIndex < MaterialSlots; ++SlotIndex)
            {
                MeshComponent->SetMaterial(SlotIndex, DynamicMaterial);
            }
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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedSafetyFillAsset(TEXT("/Game/LightWarrior/Meshes/VFX/SM_SacredCircle_SafetyFill.SM_SacredCircle_SafetyFill"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedMainRingAsset(TEXT("/Game/LightWarrior/Meshes/VFX/SM_SacredCircle_MainRing.SM_SacredCircle_MainRing"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedInnerRingAsset(TEXT("/Game/LightWarrior/Meshes/VFX/SM_SacredCircle_InnerRing_250.SM_SacredCircle_InnerRing_250"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedOuterInnerRingAsset(TEXT("/Game/LightWarrior/Meshes/VFX/SM_SacredCircle_InnerRing_375.SM_SacredCircle_InnerRing_375"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedExpansionPulseAsset(TEXT("/Game/LightWarrior/Meshes/VFX/SM_SacredCircle_ExpansionPulse.SM_SacredCircle_ExpansionPulse"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CircleMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (GeneratedSafetyFillAsset.Succeeded())
    {
        bUsingGeneratedCircleMeshes = true;
        CircleMesh->SetStaticMesh(GeneratedSafetyFillAsset.Object);
        CircleMesh->SetHiddenInGame(true);
        CircleMesh->SetVisibility(false, true);
        CircleMesh->SetCastShadow(false);
    }
    else if (CircleMeshAsset.Succeeded())
    {
        CircleMesh->SetStaticMesh(CircleMeshAsset.Object);
    }

    UStaticMesh* SegmentMesh = CircleMeshAsset.Succeeded() ? CircleMeshAsset.Object : nullptr;

    MainRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainRingMesh"));
    MainRingMesh->SetupAttachment(SceneRoot);
    MainRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -2.0f));
    MainRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MainRingMesh->SetHiddenInGame(true);
    if (GeneratedMainRingAsset.Succeeded())
    {
        MainRingMesh->SetStaticMesh(GeneratedMainRingAsset.Object);
    }

    InnerRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerRingMesh"));
    InnerRingMesh->SetupAttachment(SceneRoot);
    InnerRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -1.0f));
    InnerRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InnerRingMesh->SetHiddenInGame(true);
    if (GeneratedInnerRingAsset.Succeeded())
    {
        InnerRingMesh->SetStaticMesh(GeneratedInnerRingAsset.Object);
    }

    OuterInnerRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterInnerRingMesh"));
    OuterInnerRingMesh->SetupAttachment(SceneRoot);
    OuterInnerRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -0.5f));
    OuterInnerRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OuterInnerRingMesh->SetHiddenInGame(true);
    if (GeneratedOuterInnerRingAsset.Succeeded())
    {
        OuterInnerRingMesh->SetStaticMesh(GeneratedOuterInnerRingAsset.Object);
    }

    ExpansionPulseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExpansionPulseMesh"));
    ExpansionPulseMesh->SetupAttachment(SceneRoot);
    ExpansionPulseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -4.0f));
    ExpansionPulseMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.025f));
    ExpansionPulseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ExpansionPulseMesh->SetHiddenInGame(true);
    if (GeneratedExpansionPulseAsset.Succeeded())
    {
        ExpansionPulseMesh->SetStaticMesh(GeneratedExpansionPulseAsset.Object);
    }
    else if (CircleMeshAsset.Succeeded())
    {
        ExpansionPulseMesh->SetStaticMesh(CircleMeshAsset.Object);
    }

    for (int32 SegmentIndex = 0; SegmentIndex < 32; ++SegmentIndex)
    {
        const FName SegmentName(*FString::Printf(TEXT("BoundarySegment_%02d"), SegmentIndex));
        UStaticMeshComponent* Segment = CreateDefaultSubobject<UStaticMeshComponent>(SegmentName);
        Segment->SetupAttachment(SceneRoot);
        Segment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Segment->SetCastShadow(false);
        if (SegmentMesh)
        {
            Segment->SetStaticMesh(SegmentMesh);
        }
        BoundarySegments.Add(Segment);
    }

    for (int32 SpokeIndex = 0; SpokeIndex < 8; ++SpokeIndex)
    {
        const FName SpokeName(*FString::Printf(TEXT("SpokeSegment_%02d"), SpokeIndex));
        UStaticMeshComponent* Spoke = CreateDefaultSubobject<UStaticMeshComponent>(SpokeName);
        Spoke->SetupAttachment(SceneRoot);
        Spoke->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Spoke->SetCastShadow(false);
        if (SegmentMesh)
        {
            Spoke->SetStaticMesh(SegmentMesh);
        }
        SpokeSegments.Add(Spoke);
    }

    CircleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CircleLight"));
    CircleLight->SetupAttachment(SceneRoot);
    CircleLight->SetLightColor(FLinearColor(0.45f, 0.90f, 1.0f));
    CircleLight->SetIntensity(160.0f);
    CircleLight->SetAttenuationRadius(StartingRadius);
}

void ASacredCircle::BeginPlay()
{
    Super::BeginPlay();
    CircleMesh->SetHiddenInGame(bUsingGeneratedCircleMeshes);
    CircleMesh->SetVisibility(!bUsingGeneratedCircleMeshes, true);
    ApplyCircleColor(CircleMesh, FLinearColor(0.006f, 0.055f, 0.065f), false);
    if (!bUsingGeneratedCircleMeshes)
    {
        ApplyCircleColor(ExpansionPulseMesh, FLinearColor(0.86f, 0.58f, 0.13f), true);
    }
    for (UStaticMeshComponent* Segment : BoundarySegments)
    {
        ApplyCircleColor(Segment, FLinearColor(0.94f, 0.74f, 0.24f), false);
    }
    for (UStaticMeshComponent* Spoke : SpokeSegments)
    {
        ApplyCircleColor(Spoke, FLinearColor(0.30f, 0.86f, 0.92f), false);
    }
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
    const float VisualScale = bUsingGeneratedCircleMeshes ? CurrentRadius / 500.0f : CurrentRadius / 50.0f;
    CircleMesh->SetRelativeScale3D(FVector(VisualScale, VisualScale, bUsingGeneratedCircleMeshes ? 0.018f : 0.035f));
    if (bUsingGeneratedCircleMeshes)
    {
        CircleMesh->SetHiddenInGame(true);
        CircleMesh->SetVisibility(false, true);
        if (MainRingMesh)
        {
            MainRingMesh->SetHiddenInGame(true);
            MainRingMesh->SetVisibility(false, true);
        }
        if (InnerRingMesh)
        {
            InnerRingMesh->SetHiddenInGame(true);
            InnerRingMesh->SetVisibility(false, true);
        }
        if (OuterInnerRingMesh)
        {
            OuterInnerRingMesh->SetHiddenInGame(true);
            OuterInnerRingMesh->SetVisibility(false, true);
        }
    }
    if (MainRingMesh)
    {
        MainRingMesh->SetRelativeScale3D(FVector(VisualScale, VisualScale, VisualScale));
    }
    if (InnerRingMesh)
    {
        InnerRingMesh->SetRelativeScale3D(FVector(VisualScale, VisualScale, VisualScale));
    }
    if (OuterInnerRingMesh)
    {
        OuterInnerRingMesh->SetRelativeScale3D(FVector(VisualScale, VisualScale, VisualScale));
    }
    const int32 BoundaryCount = BoundarySegments.Num();
    const float BoundaryThickness = 0.11f;
    const float BoundaryLength = (2.0f * PI * CurrentRadius) / FMath::Max(1, BoundaryCount);
    for (int32 SegmentIndex = 0; SegmentIndex < BoundaryCount; ++SegmentIndex)
    {
        UStaticMeshComponent* Segment = BoundarySegments[SegmentIndex];
        if (!Segment)
        {
            continue;
        }
        const float Angle = (2.0f * PI * SegmentIndex) / BoundaryCount;
        Segment->SetRelativeLocation(FVector(FMath::Cos(Angle) * CurrentRadius, FMath::Sin(Angle) * CurrentRadius, -1.0f));
        Segment->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
        Segment->SetRelativeScale3D(FVector(BoundaryLength / 100.0f, BoundaryThickness, 0.018f));
    }

    const int32 SpokeCount = SpokeSegments.Num();
    const float SpokeLength = CurrentRadius * 0.72f;
    for (int32 SpokeIndex = 0; SpokeIndex < SpokeCount; ++SpokeIndex)
    {
        UStaticMeshComponent* Spoke = SpokeSegments[SpokeIndex];
        if (!Spoke)
        {
            continue;
        }
        const float Angle = (2.0f * PI * SpokeIndex) / SpokeCount;
        Spoke->SetRelativeLocation(FVector(FMath::Cos(Angle) * SpokeLength * 0.5f, FMath::Sin(Angle) * SpokeLength * 0.5f, -2.0f));
        Spoke->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(Angle), 0.0f));
        Spoke->SetRelativeScale3D(FVector(SpokeLength / 100.0f, 0.035f, 0.012f));
    }
    CircleLight->SetAttenuationRadius(CurrentRadius * 1.08f);
    CircleLight->SetIntensity(FMath::Lerp(35.0f, 180.0f, CurrentRadius / FMath::Max(1.0f, MaxRadius)));
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
    const float VisualScale = bUsingGeneratedCircleMeshes ? PulseRadius / 500.0f : PulseRadius / 50.0f;
    ExpansionPulseMesh->SetRelativeScale3D(FVector(VisualScale, VisualScale, bUsingGeneratedCircleMeshes ? VisualScale : 0.035f));
    CircleLight->SetIntensity(FMath::Lerp(1450.0f, 180.0f, Alpha));

    if (ExpansionPulseTimer <= 0.0f)
    {
        ExpansionPulseMesh->SetHiddenInGame(true);
        RefreshVisuals();
    }
}
