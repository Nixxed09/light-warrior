#include "ThunderHammerTemple.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "Engine/OverlapResult.h"
#include "LightWarriorCharacter.h"
#include "LightWarriorAudio.h"
#include "Kismet/GameplayStatics.h"
#include "SacredCircle.h"
#include "ShadowEnemy.h"

AThunderHammerTemple::AThunderHammerTemple()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    TempleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempleMesh"));
    TempleMesh->SetupAttachment(SceneRoot);
    TempleMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.6f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedPedestalAsset(TEXT("/Game/LightWarrior/Meshes/Arena/SM_LW_TemplePedestal.SM_LW_TemplePedestal"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TempleMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (GeneratedPedestalAsset.Succeeded())
    {
        bUsingGeneratedTempleMesh = true;
        TempleMesh->SetStaticMesh(GeneratedPedestalAsset.Object);
        TempleMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -48.0f));
        TempleMesh->SetRelativeScale3D(FVector(54.0f, 54.0f, 54.0f));
    }
    else if (TempleMeshAsset.Succeeded())
    {
        TempleMesh->SetStaticMesh(TempleMeshAsset.Object);
    }

    TemplePillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TemplePillarMesh"));
    TemplePillarMesh->SetupAttachment(SceneRoot);
    TemplePillarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (UStaticMesh* PillarMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Arena/SM_LW_TemplePillar.SM_LW_TemplePillar")))
    {
        TemplePillarMesh->SetStaticMesh(PillarMesh);
        TemplePillarMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));
        TemplePillarMesh->SetRelativeScale3D(FVector(50.0f, 50.0f, 50.0f));
    }
    else
    {
        TemplePillarMesh->SetHiddenInGame(true);
    }

    TempleRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempleRingMesh"));
    TempleRingMesh->SetupAttachment(SceneRoot);
    TempleRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (UStaticMesh* RingMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Arena/SM_LW_TempleRing_North.SM_LW_TempleRing_North")))
    {
        TempleRingMesh->SetStaticMesh(RingMesh);
        TempleRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 148.0f));
        TempleRingMesh->SetRelativeScale3D(FVector(50.0f, 50.0f, 50.0f));
    }
    else
    {
        TempleRingMesh->SetHiddenInGame(true);
    }

    TempleCrownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TempleCrownMesh"));
    TempleCrownMesh->SetupAttachment(SceneRoot);
    TempleCrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (UStaticMesh* HammerMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Weapons/SM_LW_ThunderHammer_Hyper3D.SM_LW_ThunderHammer_Hyper3D")))
    {
        TempleCrownMesh->SetStaticMesh(HammerMesh);
        TempleCrownMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 235.0f));
        TempleCrownMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
        TempleCrownMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.9f));
    }
    else
    {
        TempleCrownMesh->SetHiddenInGame(true);
    }

    ActivationPulseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActivationPulseMesh"));
    ActivationPulseMesh->SetupAttachment(SceneRoot);
    ActivationPulseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
    ActivationPulseMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.035f));
    ActivationPulseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ActivationPulseMesh->SetHiddenInGame(true);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PulseMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (PulseMeshAsset.Succeeded())
    {
        ActivationPulseMesh->SetStaticMesh(PulseMeshAsset.Object);
    }

    ActivationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationSphere"));
    ActivationSphere->SetupAttachment(SceneRoot);
    ActivationSphere->SetSphereRadius(260.0f);

    TempleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TempleLight"));
    TempleLight->SetupAttachment(SceneRoot);
    TempleLight->SetLightColor(FLinearColor(0.55f, 0.82f, 1.0f));
    TempleLight->SetIntensity(4500.0f);
    TempleLight->SetAttenuationRadius(1600.0f);

    TempleLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TempleLabel"));
    TempleLabel->SetupAttachment(SceneRoot);
    TempleLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 260.0f));
    TempleLabel->SetHorizontalAlignment(EHTA_Center);
    TempleLabel->SetTextRenderColor(FColor(255, 218, 96));
    TempleLabel->SetWorldSize(92.0f);
    TempleLabel->SetText(FText::FromString(TEXT("THUNDER HAMMER")));
    TempleLabel->SetHiddenInGame(true);
}

void AThunderHammerTemple::BeginPlay()
{
    Super::BeginPlay();
    ActivationSphere->OnComponentBeginOverlap.AddDynamic(this, &AThunderHammerTemple::OnTempleOverlap);
    SetTempleUnlocked(bUnlocked);
}

void AThunderHammerTemple::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (LockedPulseTimer > 0.0f)
    {
        LockedPulseTimer = FMath::Max(0.0f, LockedPulseTimer - DeltaSeconds);
        const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 18.0f);
        TempleLight->SetLightColor(FLinearColor(0.34f, 0.72f, 1.0f));
        TempleLight->SetIntensity(FMath::Lerp(6200.0f, 11500.0f, Pulse));

        if (LockedPulseTimer <= 0.0f && !bUnlocked)
        {
            TempleLight->SetLightColor(FLinearColor(0.25f, 0.46f, 0.68f));
            TempleLight->SetIntensity(2600.0f);
        }
    }

    if (ActivationPulseTimer <= 0.0f || !ActivationPulseMesh)
    {
        return;
    }

    ActivationPulseTimer = FMath::Max(0.0f, ActivationPulseTimer - DeltaSeconds);
    const float Alpha = 1.0f - ActivationPulseTimer / FMath::Max(0.01f, ActivationPulseDuration);
    const float RingScale = FMath::Lerp(2.6f, 24.0f, Alpha);
    const float Flash = FMath::Square(1.0f - Alpha);
    ActivationPulseMesh->SetRelativeScale3D(FVector(RingScale, RingScale, 0.035f));
    TempleLight->SetIntensity(FMath::Lerp(34000.0f, 12000.0f, Alpha) + 8000.0f * Flash);

    if (ActivationPulseTimer <= 0.0f)
    {
        ActivationPulseMesh->SetHiddenInGame(true);
        TempleLight->SetIntensity(12000.0f);
    }
}

void AThunderHammerTemple::SetTempleUnlocked(bool bNewUnlocked)
{
    bUnlocked = bNewUnlocked;

    if (bUnlocked)
    {
        TempleLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.18f));
        TempleLight->SetIntensity(12000.0f);
        TempleLight->SetAttenuationRadius(2600.0f);
        TempleMesh->SetRelativeScale3D(bUsingGeneratedTempleMesh ? FVector(60.0f, 60.0f, 60.0f) : FVector(2.18f, 2.18f, 2.82f));
        if (TemplePillarMesh)
        {
            TemplePillarMesh->SetHiddenInGame(false);
        }
        if (TempleRingMesh)
        {
            TempleRingMesh->SetHiddenInGame(false);
        }
        if (TempleCrownMesh)
        {
            TempleCrownMesh->SetHiddenInGame(false);
        }
        TempleLabel->SetText(FText::FromString(TEXT("THUNDER HAMMER")));
        return;
    }

    TempleLight->SetLightColor(FLinearColor(0.25f, 0.46f, 0.68f));
    TempleLight->SetIntensity(2600.0f);
    TempleLight->SetAttenuationRadius(1250.0f);
    TempleMesh->SetRelativeScale3D(bUsingGeneratedTempleMesh ? FVector(52.0f, 52.0f, 52.0f) : FVector(1.92f, 1.92f, 2.42f));
    if (TemplePillarMesh)
    {
        TemplePillarMesh->SetHiddenInGame(false);
    }
    if (TempleRingMesh)
    {
        TempleRingMesh->SetHiddenInGame(true);
    }
    if (TempleCrownMesh)
    {
        TempleCrownMesh->SetHiddenInGame(true);
    }
    TempleLabel->SetText(FText::FromString(TEXT("RESTORE WELLS")));
}

void AThunderHammerTemple::OnTempleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    ActivateTemple(Character);
}

void AThunderHammerTemple::ActivateTemple(ALightWarriorCharacter* Character)
{
    if (!Character || bActivated)
    {
        return;
    }

    if (!bUnlocked)
    {
        LockedPulseTimer = 0.55f;
        return;
    }

    bActivated = true;
    Character->GrantThunderHammer(HammerDuration);
    TempleLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.18f));
    TempleLight->SetIntensity(16000.0f);
    TempleLight->SetAttenuationRadius(2600.0f);
    TempleMesh->SetRelativeScale3D(bUsingGeneratedTempleMesh ? FVector(64.0f, 64.0f, 64.0f) : FVector(2.35f, 2.35f, 3.05f));
    if (TempleRingMesh)
    {
        TempleRingMesh->SetHiddenInGame(false);
    }
    if (TempleCrownMesh)
    {
        TempleCrownMesh->SetHiddenInGame(false);
    }
    TempleLabel->SetText(FText::FromString(TEXT("HAMMER AWAKENED")));
    ActivationPulseTimer = ActivationPulseDuration;
    ActivationPulseMesh->SetHiddenInGame(false);
    ULightWarriorAudio::PlaySfx(this, ELightWarriorSfx::TempleActivate, GetActorLocation(), 1.05f);

    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        It->ExpandFromCombat(ActivationExpansionAmount);
        break;
    }

    TArray<FOverlapResult> Overlaps;
    const FCollisionShape Sphere = FCollisionShape::MakeSphere(ActivationDamageRadius);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ThunderHammerActivation), false, this);
    if (GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        Sphere,
        QueryParams))
    {
        for (const FOverlapResult& Result : Overlaps)
        {
            AShadowEnemy* Enemy = Cast<AShadowEnemy>(Result.GetActor());
            if (Enemy)
            {
                UGameplayStatics::ApplyDamage(Enemy, ActivationDamage, Character->GetController(), Character, UDamageType::StaticClass());
            }
        }
    }
}
