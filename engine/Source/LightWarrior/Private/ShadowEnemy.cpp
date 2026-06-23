#include "ShadowEnemy.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/PointLight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorCharacter.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
void ApplyShadowColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color, bool bEmissive)
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

AShadowEnemy::AShadowEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCharacterMovement()->MaxWalkSpeed = 285.0f;

    ShadowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowMesh"));
    ShadowMesh->SetupAttachment(RootComponent);
    ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -25.0f));
    ShadowMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
    ShadowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (ShadowMeshAsset.Succeeded())
    {
        ShadowMesh->SetStaticMesh(ShadowMeshAsset.Object);
    }
    ApplyShadowColor(ShadowMesh, FLinearColor(0.08f, 0.015f, 0.16f), true);

    AttackTellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttackTellMesh"));
    AttackTellMesh->SetupAttachment(RootComponent);
    AttackTellMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -78.0f));
    AttackTellMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.02f));
    AttackTellMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AttackTellMesh->SetHiddenInGame(true);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TellMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (TellMeshAsset.Succeeded())
    {
        AttackTellMesh->SetStaticMesh(TellMeshAsset.Object);
    }
    ApplyShadowColor(AttackTellMesh, FLinearColor(1.0f, 0.08f, 0.32f), true);

    ShadowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShadowLight"));
    ShadowLight->SetupAttachment(RootComponent);
    ShadowLight->SetLightColor(FLinearColor(0.18f, 0.04f, 0.34f));
    ShadowLight->SetIntensity(1600.0f);
    ShadowLight->SetAttenuationRadius(520.0f);

    ShadowLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ShadowLabel"));
    ShadowLabel->SetupAttachment(RootComponent);
    ShadowLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
    ShadowLabel->SetHorizontalAlignment(EHTA_Center);
    ShadowLabel->SetTextRenderColor(FColor(198, 82, 255));
    ShadowLabel->SetWorldSize(58.0f);
    ShadowLabel->SetText(FText::FromString(TEXT("SHADOW")));
    ShadowLabel->SetHiddenInGame(true);
}

void AShadowEnemy::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

void AShadowEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaSeconds);
    UpdateAttackTell(DeltaSeconds);
    UpdateHitFeedback(DeltaSeconds);

    ALightWarriorCharacter* Player = FindPlayer();
    if (!Player)
    {
        return;
    }

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size2D();

    if (bAttackWindingUp)
    {
        if (Distance <= AttackRange * 1.35f && AttackWindupRemaining <= 0.0f)
        {
            FinishAttack(Player);
        }
        return;
    }

    if (Distance > AttackRange)
    {
        AddMovementInput(ToPlayer.GetSafeNormal2D(), 1.0f);
        return;
    }

    if (AttackCooldownRemaining <= 0.0f)
    {
        StartAttackWindup();
    }
}

float AShadowEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health -= AppliedDamage;
    HitFeedbackRemaining = 0.22f;
    ApplyShadowColor(ShadowMesh, FLinearColor(0.72f, 0.18f, 1.0f), true);
    ShadowLight->SetIntensity(7200.0f);
    ShadowLight->SetLightColor(FLinearColor(0.85f, 0.20f, 1.0f));

    if (Health <= 0.0f)
    {
        SpawnDeathBurst();
        Destroy();
    }

    return AppliedDamage;
}

ALightWarriorCharacter* AShadowEnemy::FindPlayer() const
{
    return Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AShadowEnemy::StartAttackWindup()
{
    bAttackWindingUp = true;
    AttackWindupRemaining = AttackWindupSeconds;
    AttackTellMesh->SetHiddenInGame(false);
    AttackTellMesh->SetRelativeScale3D(FVector(2.1f, 2.1f, 0.025f));
    ShadowLight->SetLightColor(FLinearColor(1.0f, 0.05f, 0.28f));
    ShadowLight->SetIntensity(6200.0f);
}

void AShadowEnemy::FinishAttack(ALightWarriorCharacter* Player)
{
    bAttackWindingUp = false;
    AttackTellMesh->SetHiddenInGame(true);
    AttackCooldownRemaining = AttackCooldown;
    ShadowLight->SetLightColor(FLinearColor(0.18f, 0.04f, 0.34f));
    ShadowLight->SetIntensity(1800.0f);

    if (Player)
    {
        UGameplayStatics::ApplyDamage(Player, ContactDamage, GetController(), this, UDamageType::StaticClass());
    }
}

void AShadowEnemy::UpdateAttackTell(float DeltaSeconds)
{
    if (!bAttackWindingUp)
    {
        return;
    }

    AttackWindupRemaining = FMath::Max(0.0f, AttackWindupRemaining - DeltaSeconds);
    const float Alpha = 1.0f - AttackWindupRemaining / FMath::Max(0.01f, AttackWindupSeconds);
    const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 18.0f);
    const float TellScale = FMath::Lerp(1.25f, 2.65f, Alpha);
    AttackTellMesh->SetRelativeScale3D(FVector(TellScale, TellScale, 0.025f));
    ShadowLight->SetIntensity(FMath::Lerp(4200.0f, 9200.0f, FMath::Max(Alpha, Pulse * 0.8f)));
}

void AShadowEnemy::UpdateHitFeedback(float DeltaSeconds)
{
    if (HitFeedbackRemaining <= 0.0f)
    {
        return;
    }

    HitFeedbackRemaining = FMath::Max(0.0f, HitFeedbackRemaining - DeltaSeconds);
    if (HitFeedbackRemaining <= 0.0f && !bAttackWindingUp)
    {
        ApplyShadowColor(ShadowMesh, FLinearColor(0.08f, 0.015f, 0.16f), true);
        ShadowLight->SetLightColor(FLinearColor(0.18f, 0.04f, 0.34f));
        ShadowLight->SetIntensity(1600.0f);
    }
}

void AShadowEnemy::SpawnDeathBurst() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APointLight* Burst = World->SpawnActor<APointLight>(GetActorLocation() + FVector(0.0f, 0.0f, 120.0f), FRotator::ZeroRotator, SpawnParams);
    if (Burst && Burst->PointLightComponent)
    {
        Burst->PointLightComponent->SetLightColor(FLinearColor(1.0f, 0.82f, 0.24f));
        Burst->PointLightComponent->SetIntensity(22000.0f);
        Burst->PointLightComponent->SetAttenuationRadius(1500.0f);
        Burst->SetLifeSpan(0.42f);
        Burst->SetActorLabel(TEXT("LW_ShadowDeathBurst"));
    }
}
