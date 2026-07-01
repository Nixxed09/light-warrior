#include "LightShardPickup.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "LightWarriorCharacter.h"
#include "LightWarriorProgressionComponent.h"
#include "SacredCircle.h"
#include "UObject/ConstructorHelpers.h"

ALightShardPickup::ALightShardPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    ShardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShardMesh"));
    ShardMesh->SetupAttachment(SceneRoot);
    ShardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShardMesh->SetRelativeScale3D(FVector(0.78f));

    if (UStaticMesh* GeneratedShardMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Pickups/SM_LW_LightShard.SM_LW_LightShard")))
    {
        ShardMesh->SetStaticMesh(GeneratedShardMesh);
    }
    else
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (FallbackMeshAsset.Succeeded())
        {
            ShardMesh->SetStaticMesh(FallbackMeshAsset.Object);
        }
    }

    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    PickupSphere->SetupAttachment(SceneRoot);
    PickupSphere->SetSphereRadius(120.0f);
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    ShardLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShardLight"));
    ShardLight->SetupAttachment(SceneRoot);
    ShardLight->SetLightColor(FLinearColor(1.0f, 0.88f, 0.28f));
    ShardLight->SetIntensity(2600.0f);
    ShardLight->SetAttenuationRadius(620.0f);
}

void ALightShardPickup::BeginPlay()
{
    Super::BeginPlay();
    PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ALightShardPickup::OnPickupOverlap);
}

void ALightShardPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    LifeTimeSeconds += DeltaSeconds;
    const float Bob = FMath::Sin(LifeTimeSeconds * 3.0f) * 18.0f;
    ShardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 82.0f + Bob));
    ShardMesh->AddLocalRotation(FRotator(0.0f, 96.0f * DeltaSeconds, 0.0f));

    const float Pulse = 0.5f + 0.5f * FMath::Sin(LifeTimeSeconds * 6.0f);
    ShardLight->SetIntensity(FMath::Lerp(1800.0f, 4400.0f, Pulse));
}

void ALightShardPickup::OnPickupOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    Collect(Cast<ALightWarriorCharacter>(OtherActor));
}

void ALightShardPickup::Collect(ALightWarriorCharacter* Character)
{
    if (!Character || bCollected)
    {
        return;
    }

    bCollected = true;

    if (ULightWarriorProgressionComponent* Progression = Character->GetProgressionComponent())
    {
        Progression->AddResource(ELightWarriorResource::Light, LightReward);
        Progression->AddResource(ELightWarriorResource::Courage, CourageReward);
        Progression->AddResource(ELightWarriorResource::ResonanceShards, ResonanceShardReward);
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_SHARD_PICKUP light=%.0f courage=%.0f shards=%.0f total_shards=%.0f"),
            LightReward,
            CourageReward,
            ResonanceShardReward,
            Progression->GetResource(ELightWarriorResource::ResonanceShards));
    }

    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        It->ExpandFromCombat(CircleExpansionReward);
        break;
    }

    Destroy();
}
