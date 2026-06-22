#include "ThunderHammerTemple.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "LightWarriorCharacter.h"
#include "SacredCircle.h"

AThunderHammerTemple::AThunderHammerTemple()
{
    PrimaryActorTick.bCanEverTick = false;

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
}

void AThunderHammerTemple::BeginPlay()
{
    Super::BeginPlay();
    ActivationSphere->OnComponentBeginOverlap.AddDynamic(this, &AThunderHammerTemple::OnTempleOverlap);
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

    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        It->ExpandFromCombat(ActivationExpansionAmount);
        break;
    }
}
