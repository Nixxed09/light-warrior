#include "LightWell.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "LightWarriorCharacter.h"
#include "SacredCircle.h"

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
        WellLight->SetIntensity(FMath::Lerp(2800.0f, 7200.0f, Pulse));

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
    WellLight->SetIntensity(FMath::Lerp(900.0f, 6500.0f, LightAlpha));
    WellLight->SetAttenuationRadius(FMath::Lerp(900.0f, 1800.0f, LightAlpha));
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
    WellLight->SetLightColor(FLinearColor(1.0f, 0.92f, 0.45f));
    WellLight->SetIntensity(14500.0f);
    WellLight->SetAttenuationRadius(3000.0f);
    WellLabel->SetText(FText::FromString(TEXT("RESTORED")));
    WellLabel->SetTextRenderColor(FColor(245, 245, 180));

    ExpandSacredCircle();
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
