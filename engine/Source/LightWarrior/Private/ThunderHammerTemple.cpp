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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> TempleMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (TempleMeshAsset.Succeeded())
    {
        TempleMesh->SetStaticMesh(TempleMeshAsset.Object);
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
}

void AThunderHammerTemple::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

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

    bActivated = true;
    Character->GrantThunderHammer(HammerDuration);
    TempleLight->SetLightColor(FLinearColor(1.0f, 0.72f, 0.18f));
    TempleLight->SetIntensity(16000.0f);
    TempleLight->SetAttenuationRadius(2600.0f);
    TempleMesh->SetRelativeScale3D(FVector(2.35f, 2.35f, 3.05f));
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
