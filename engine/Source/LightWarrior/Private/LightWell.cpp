#include "LightWell.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "LightWarriorCharacter.h"
#include "LightWarriorAudio.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "SacredCircle.h"
#include "ShadowEnemy.h"

namespace
{
void ApplyLightWellMeshColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color, bool bEmissive)
{
    if (!MeshComponent)
    {
        return;
    }

    const TCHAR* MaterialPath = bEmissive
        ? TEXT("/Engine/ArtTools/RenderToTexture/Materials/Debug/M_Emissive_Color.M_Emissive_Color")
        : TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

    UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
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

ALightWell::ALightWell()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    WellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WellMesh"));
    WellMesh->SetupAttachment(SceneRoot);
    WellMesh->SetRelativeScale3D(FVector(1.35f, 1.35f, 0.45f));
    WellMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> WellMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (WellMeshAsset.Succeeded())
    {
        WellMesh->SetStaticMesh(WellMeshAsset.Object);
    }
    ApplyLightWellMeshColor(WellMesh, FLinearColor(0.34f, 0.95f, 1.0f), true);

    RestorationRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RestorationRingMesh"));
    RestorationRingMesh->SetupAttachment(SceneRoot);
    RestorationRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    RestorationRingMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.025f));
    RestorationRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RestorationRingMesh->SetHiddenInGame(true);
    if (WellMeshAsset.Succeeded())
    {
        RestorationRingMesh->SetStaticMesh(WellMeshAsset.Object);
    }
    ApplyLightWellMeshColor(RestorationRingMesh, FLinearColor(0.84f, 0.62f, 0.18f), false);

    PurificationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PurificationSphere"));
    PurificationSphere->SetupAttachment(SceneRoot);
    PurificationSphere->SetSphereRadius(PurificationRadius);

    WellLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("WellLight"));
    WellLight->SetupAttachment(SceneRoot);
    WellLight->SetLightColor(FLinearColor(0.70f, 0.95f, 1.0f));
    WellLight->SetIntensity(900.0f);
    WellLight->SetAttenuationRadius(900.0f);

    WellLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("WellLabel"));
    WellLabel->SetupAttachment(SceneRoot);
    WellLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
    WellLabel->SetHorizontalAlignment(EHTA_Center);
    WellLabel->SetTextRenderColor(FColor(150, 235, 255));
    WellLabel->SetWorldSize(88.0f);
    WellLabel->SetText(FText::FromString(TEXT("LIGHT WELL")));
    WellLabel->SetHiddenInGame(true);
}

void ALightWell::BeginPlay()
{
    Super::BeginPlay();

    PurificationSphere->SetSphereRadius(PurificationRadius);
    PurificationSphere->OnComponentBeginOverlap.AddDynamic(this, &ALightWell::OnPurificationBegin);
    PurificationSphere->OnComponentEndOverlap.AddDynamic(this, &ALightWell::OnPurificationEnd);
    OnPurificationChanged.Broadcast(GetPurification01(), IsBeingPurified());
}

void ALightWell::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bPurified)
    {
        return;
    }

    if (bPlayerInside)
    {
        const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 7.0f);
        WellLight->SetIntensity(FMath::Lerp(1800.0f, 4200.0f, Pulse));

        TimeSincePlayerLeft = 0.0f;
        SetPurificationProgress(PurificationProgress + DeltaSeconds);

        if (PurificationProgress >= PurificationSeconds)
        {
            CompletePurification();
        }

        return;
    }

    TimeSincePlayerLeft += DeltaSeconds;
    if (TimeSincePlayerLeft >= DecayDelaySeconds && PurificationProgress > 0.0f)
    {
        const float DecayRate = PurificationSeconds / FMath::Max(0.01f, DecaySeconds);
        SetPurificationProgress(PurificationProgress - DecayRate * DeltaSeconds);
    }
}

float ALightWell::GetPurification01() const
{
    return FMath::Clamp(PurificationProgress / FMath::Max(0.01f, PurificationSeconds), 0.0f, 1.0f);
}

void ALightWell::OnPurificationBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(OtherActor);
    if (!Character || bPurified)
    {
        return;
    }

    PurifyingCharacter = Character;
    SetPlayerInside(true);

    if (!bHasStartedPurification)
    {
        bHasStartedPurification = true;
        OnPurificationStarted.Broadcast(this);
        WellLabel->SetText(FText::FromString(TEXT("HOLD THE WELL")));
        WellLabel->SetTextRenderColor(FColor(255, 232, 128));
    }
}

void ALightWell::OnPurificationEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (OtherActor != PurifyingCharacter)
    {
        return;
    }

    PurifyingCharacter = nullptr;
    SetPlayerInside(false);
}

void ALightWell::SetPlayerInside(bool bNewPlayerInside)
{
    if (bPlayerInside == bNewPlayerInside)
    {
        return;
    }

    bPlayerInside = bNewPlayerInside;
    OnPurificationChanged.Broadcast(GetPurification01(), IsBeingPurified());
}

void ALightWell::SetPurificationProgress(float NewProgress)
{
    const float ClampedProgress = FMath::Clamp(NewProgress, 0.0f, PurificationSeconds);
    if (FMath::IsNearlyEqual(PurificationProgress, ClampedProgress))
    {
        return;
    }

    PurificationProgress = ClampedProgress;
    OnPurificationChanged.Broadcast(GetPurification01(), IsBeingPurified());

    const float LightAlpha = GetPurification01();
    WellLight->SetIntensity(FMath::Lerp(650.0f, 3600.0f, LightAlpha));
    WellLight->SetAttenuationRadius(FMath::Lerp(720.0f, 1250.0f, LightAlpha));
}

void ALightWell::CompletePurification()
{
    if (bPurified)
    {
        return;
    }

    bPurified = true;
    PurificationProgress = PurificationSeconds;
    WellMesh->SetRelativeScale3D(FVector(1.85f, 1.85f, 0.62f));
    ApplyLightWellMeshColor(WellMesh, FLinearColor(0.94f, 0.68f, 0.20f), false);
    RestorationRingMesh->SetHiddenInGame(false);
    RestorationRingMesh->SetRelativeScale3D(FVector(3.2f, 3.2f, 0.025f));
    WellLight->SetLightColor(FLinearColor(1.0f, 0.92f, 0.45f));
    WellLight->SetIntensity(4200.0f);
    WellLight->SetAttenuationRadius(1450.0f);
    WellLabel->SetText(FText::FromString(TEXT("FIELD EXPANDS")));
    WellLabel->SetTextRenderColor(FColor(245, 245, 180));
    ULightWarriorAudio::PlaySfx(this, ELightWarriorSfx::CircleExpand, GetActorLocation(), 0.92f);

    ExpandSacredCircle();
    RepelNearbyShadows();
    OnPurificationChanged.Broadcast(1.0f, false);
    OnPurified.Broadcast(this);
}

void ALightWell::ExpandSacredCircle() const
{
    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        It->ExpandFromCombat(CircleExpansionOnPurified);
        return;
    }
}

void ALightWell::RepelNearbyShadows()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector BurstOrigin = GetActorLocation();
    for (TActorIterator<AShadowEnemy> It(World); It; ++It)
    {
        AShadowEnemy* Enemy = *It;
        if (!Enemy)
        {
            continue;
        }

        const FVector ToEnemy = Enemy->GetActorLocation() - BurstOrigin;
        if (ToEnemy.SizeSquared2D() > FMath::Square(RestorationBurstRadius))
        {
            continue;
        }

        const FVector PushDirection = FVector(ToEnemy.X, ToEnemy.Y, 0.0f).GetSafeNormal();
        Enemy->LaunchCharacter(PushDirection * 1250.0f + FVector(0.0f, 0.0f, 260.0f), true, true);
        Enemy->TakeDamage(RestorationBurstDamage, FDamageEvent(), nullptr, this);
    }
}
