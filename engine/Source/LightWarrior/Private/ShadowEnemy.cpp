#include "ShadowEnemy.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/PointLight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorAudio.h"
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
    ApplyArchetypeVisuals();
    Health = MaxHealth;
}

void AShadowEnemy::ConfigureArchetype(EShadowEnemyArchetype NewArchetype)
{
    Archetype = NewArchetype;

    switch (Archetype)
    {
    case EShadowEnemyArchetype::Berserker:
        MaxHealth = 145.0f;
        ContactDamage = 18.0f;
        AttackRange = 245.0f;
        AttackCooldown = 3.45f;
        AttackWindupSeconds = 0.95f;
        GetCharacterMovement()->MaxWalkSpeed = 215.0f;
        BaseShadowColor = FLinearColor(0.12f, 0.005f, 0.07f);
        TellColor = FLinearColor(1.0f, 0.08f, 0.12f);
        BurstColor = FLinearColor(1.0f, 0.50f, 0.18f);
        break;

    case EShadowEnemyArchetype::ShadowImp:
    default:
        MaxHealth = 55.0f;
        ContactDamage = 7.0f;
        AttackRange = 165.0f;
        AttackCooldown = 2.25f;
        AttackWindupSeconds = 0.48f;
        GetCharacterMovement()->MaxWalkSpeed = 360.0f;
        BaseShadowColor = FLinearColor(0.08f, 0.015f, 0.16f);
        TellColor = FLinearColor(0.82f, 0.10f, 1.0f);
        BurstColor = FLinearColor(1.0f, 0.82f, 0.24f);
        break;
    }

    Health = MaxHealth;
    ApplyArchetypeVisuals();
}

void AShadowEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaSeconds);
    UpdatePressurePreview(DeltaSeconds);
    UpdateAttackTell(DeltaSeconds);
    UpdateHitFeedback(DeltaSeconds);

    if (bPressurePreviewActive)
    {
        return;
    }

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
    PulseHitFeedback(0.22f);

    if (Health <= 0.0f)
    {
        ULightWarriorAudio::PlaySfx(this, ELightWarriorSfx::ShadowDissolve, GetActorLocation(), Archetype == EShadowEnemyArchetype::Berserker ? 1.05f : 0.78f);
        SpawnDeathBurst();
        Destroy();
    }

    return AppliedDamage;
}

void AShadowEnemy::BeginPressurePreview(float DurationSeconds)
{
    bPressurePreviewActive = true;
    PressurePreviewDuration = FMath::Max(0.1f, DurationSeconds);
    PressurePreviewRemaining = PressurePreviewDuration;
    AttackTellMesh->SetHiddenInGame(false);
    ShadowLight->SetLightColor(TellColor);
    ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 12000.0f : 8200.0f);
}

void AShadowEnemy::PulseHitFeedback(float DurationSeconds)
{
    HitFeedbackRemaining = FMath::Max(0.05f, DurationSeconds);
    ApplyShadowColor(ShadowMesh, FLinearColor(0.72f, 0.18f, 1.0f), true);
    ShadowLight->SetIntensity(7200.0f);
    ShadowLight->SetLightColor(FLinearColor(0.85f, 0.20f, 1.0f));
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
    const float StartingTellScale = Archetype == EShadowEnemyArchetype::Berserker ? 3.0f : 1.75f;
    AttackTellMesh->SetRelativeScale3D(FVector(StartingTellScale, StartingTellScale, 0.025f));
    ShadowLight->SetLightColor(TellColor);
    ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 8800.0f : 6200.0f);
}

void AShadowEnemy::FinishAttack(ALightWarriorCharacter* Player)
{
    bAttackWindingUp = false;
    AttackTellMesh->SetHiddenInGame(true);
    AttackCooldownRemaining = AttackCooldown;
    ShadowLight->SetLightColor(BaseShadowColor);
    ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 2400.0f : 1800.0f);

    if (Player)
    {
        UGameplayStatics::ApplyDamage(Player, ContactDamage, GetController(), this, UDamageType::StaticClass());
        if (Archetype == EShadowEnemyArchetype::Berserker)
        {
            const FVector LaunchDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
            Player->LaunchCharacter(LaunchDirection * 720.0f + FVector(0.0f, 0.0f, 130.0f), true, true);
        }
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
    const float MinTellScale = Archetype == EShadowEnemyArchetype::Berserker ? 2.45f : 1.15f;
    const float MaxTellScale = Archetype == EShadowEnemyArchetype::Berserker ? 4.65f : 2.65f;
    const float TellScale = FMath::Lerp(MinTellScale, MaxTellScale, Alpha);
    AttackTellMesh->SetRelativeScale3D(FVector(TellScale, TellScale, 0.025f));
    ShadowLight->SetIntensity(FMath::Lerp(4200.0f, Archetype == EShadowEnemyArchetype::Berserker ? 13200.0f : 9200.0f, FMath::Max(Alpha, Pulse * 0.8f)));
}

void AShadowEnemy::UpdatePressurePreview(float DeltaSeconds)
{
    if (!bPressurePreviewActive)
    {
        return;
    }

    PressurePreviewRemaining = FMath::Max(0.0f, PressurePreviewRemaining - DeltaSeconds);
    const float Alpha = 1.0f - PressurePreviewRemaining / FMath::Max(0.01f, PressurePreviewDuration);
    const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 14.0f);
    const float BaseScale = Archetype == EShadowEnemyArchetype::Berserker ? 3.8f : 2.2f;
    const float TellScale = BaseScale + Pulse * (Archetype == EShadowEnemyArchetype::Berserker ? 1.35f : 0.85f) + Alpha * 0.65f;
    AttackTellMesh->SetHiddenInGame(false);
    AttackTellMesh->SetRelativeScale3D(FVector(TellScale, TellScale, 0.03f));
    ShadowLight->SetLightColor(TellColor);
    ShadowLight->SetIntensity(FMath::Lerp(6200.0f, Archetype == EShadowEnemyArchetype::Berserker ? 14500.0f : 9800.0f, Pulse));

    if (PressurePreviewRemaining <= 0.0f)
    {
        bPressurePreviewActive = false;
        AttackTellMesh->SetHiddenInGame(true);
        ShadowLight->SetLightColor(BaseShadowColor);
        ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 2200.0f : 1600.0f);
        AttackCooldownRemaining = FMath::Min(AttackCooldownRemaining, 0.35f);
    }
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
        ApplyShadowColor(ShadowMesh, BaseShadowColor, true);
        ShadowLight->SetLightColor(BaseShadowColor);
        ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 2200.0f : 1600.0f);
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
        Burst->PointLightComponent->SetLightColor(BurstColor);
        Burst->PointLightComponent->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 32000.0f : 22000.0f);
        Burst->PointLightComponent->SetAttenuationRadius(Archetype == EShadowEnemyArchetype::Berserker ? 2100.0f : 1500.0f);
        Burst->SetLifeSpan(Archetype == EShadowEnemyArchetype::Berserker ? 0.72f : 0.42f);
    }
}

void AShadowEnemy::ApplyArchetypeVisuals()
{
    if (Archetype == EShadowEnemyArchetype::Berserker)
    {
        UStaticMesh* BerserkerMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Enemies/SM_Berserker_ChestPlate.SM_Berserker_ChestPlate"));
        if (BerserkerMesh)
        {
            ShadowMesh->SetStaticMesh(BerserkerMesh);
            ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
            ShadowMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
        }
        else
        {
            ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
            ShadowMesh->SetRelativeScale3D(FVector(1.35f, 1.05f, 1.85f));
        }
        AttackTellMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.02f));
        ShadowLight->SetAttenuationRadius(780.0f);
        ShadowLabel->SetText(FText::FromString(TEXT("BERSERKER")));
        ShadowLabel->SetTextRenderColor(FColor(255, 90, 130));
        ShadowLabel->SetWorldSize(68.0f);
    }
    else
    {
        UStaticMesh* ImpMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Enemies/SM_LW_ShadowImp_Hyper3D.SM_LW_ShadowImp_Hyper3D"));
        if (ImpMesh)
        {
            ShadowMesh->SetStaticMesh(ImpMesh);
            ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
            ShadowMesh->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.42f));
        }
        else
        {
            ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -36.0f));
            ShadowMesh->SetRelativeScale3D(FVector(0.66f, 0.66f, 0.92f));
        }
        AttackTellMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.02f));
        ShadowLight->SetAttenuationRadius(520.0f);
        ShadowLabel->SetText(FText::FromString(TEXT("IMP")));
        ShadowLabel->SetTextRenderColor(FColor(206, 92, 255));
        ShadowLabel->SetWorldSize(58.0f);
    }

    ApplyShadowColor(ShadowMesh, BaseShadowColor, true);
    ApplyShadowColor(AttackTellMesh, TellColor, true);
    ShadowLight->SetLightColor(BaseShadowColor);
    ShadowLight->SetIntensity(Archetype == EShadowEnemyArchetype::Berserker ? 2200.0f : 1600.0f);
}
