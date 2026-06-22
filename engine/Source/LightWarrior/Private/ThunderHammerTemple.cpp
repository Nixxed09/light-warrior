#include "ThunderHammerTemple.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
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

    ActivationSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationSphere"));
    ActivationSphere->SetupAttachment(SceneRoot);
    ActivationSphere->SetSphereRadius(260.0f);

    TempleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TempleLight"));
    TempleLight->SetupAttachment(SceneRoot);
    TempleLight->SetLightColor(FLinearColor(0.55f, 0.82f, 1.0f));
    TempleLight->SetIntensity(4500.0f);
    TempleLight->SetAttenuationRadius(1600.0f);
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

    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        It->ExpandFromCombat(ActivationExpansionAmount);
        break;
    }
}
