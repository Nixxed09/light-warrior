#include "LightWarriorCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DamageEvents.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorProgressionComponent.h"
#include "LightWarriorAudio.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SacredCircle.h"
#include "ShadowEnemy.h"

ALightWarriorCharacter::ALightWarriorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 900.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 760.0f;
    GetCharacterMovement()->MaxAcceleration = 5200.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = NormalBrakingDeceleration;
    GetCharacterMovement()->GroundFriction = NormalGroundFriction;
    GetCharacterMovement()->BrakingFrictionFactor = 1.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 900.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 112.0f);
    CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 74.0f);
    CameraBoom->SetRelativeRotation(FRotator(-34.0f, 0.0f, 0.0f));
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 13.0f;
    CameraBoom->CameraLagMaxDistance = 95.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    HeroMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroMesh"));
    HeroMesh->SetupAttachment(RootComponent);
    HeroMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
    HeroMesh->SetRelativeScale3D(FVector(1.05f, 1.05f, 1.65f));
    HeroMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroMesh->SetHiddenInGame(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> GeneratedHeroAsset(TEXT("/Game/LightWarrior/Meshes/Characters/SM_LW_Hero_Hyper3D.SM_LW_Hero_Hyper3D"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroSphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroCubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (GeneratedHeroAsset.Succeeded())
    {
        bUsingGeneratedHeroMesh = true;
        HeroMesh->SetStaticMesh(GeneratedHeroAsset.Object);
        HeroMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -98.0f));
        HeroMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 90.0f));
        HeroMesh->SetRelativeScale3D(FVector(0.019f, 0.019f, 0.019f));
    }
    else if (HeroMeshAsset.Succeeded())
    {
        HeroMesh->SetStaticMesh(HeroMeshAsset.Object);
    }

    HeroHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroHeadMesh"));
    HeroHeadMesh->SetupAttachment(RootComponent);
    HeroHeadMesh->SetRelativeLocation(FVector(30.0f, 0.0f, 122.0f));
    HeroHeadMesh->SetRelativeScale3D(FVector(0.82f, 0.82f, 0.82f));
    HeroHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroHeadMesh->SetHiddenInGame(false);
    if (HeroSphereAsset.Succeeded())
    {
        HeroHeadMesh->SetStaticMesh(HeroSphereAsset.Object);
    }
    HeroHeadMesh->SetHiddenInGame(bUsingGeneratedHeroMesh);

    HeroLeftEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroLeftEyeMesh"));
    HeroLeftEyeMesh->SetupAttachment(RootComponent);
    HeroLeftEyeMesh->SetRelativeLocation(FVector(103.0f, -20.0f, 136.0f));
    HeroLeftEyeMesh->SetRelativeScale3D(FVector(0.12f, 0.08f, 0.12f));
    HeroLeftEyeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (HeroSphereAsset.Succeeded())
    {
        HeroLeftEyeMesh->SetStaticMesh(HeroSphereAsset.Object);
    }
    HeroLeftEyeMesh->SetHiddenInGame(bUsingGeneratedHeroMesh);

    HeroRightEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroRightEyeMesh"));
    HeroRightEyeMesh->SetupAttachment(RootComponent);
    HeroRightEyeMesh->SetRelativeLocation(FVector(103.0f, 20.0f, 136.0f));
    HeroRightEyeMesh->SetRelativeScale3D(FVector(0.12f, 0.08f, 0.12f));
    HeroRightEyeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (HeroSphereAsset.Succeeded())
    {
        HeroRightEyeMesh->SetStaticMesh(HeroSphereAsset.Object);
    }
    HeroRightEyeMesh->SetHiddenInGame(bUsingGeneratedHeroMesh);

    HeroLeftArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroLeftArmMesh"));
    HeroLeftArmMesh->SetupAttachment(RootComponent);
    HeroLeftArmMesh->SetRelativeLocation(FVector(14.0f, -74.0f, 44.0f));
    HeroLeftArmMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -28.0f));
    HeroLeftArmMesh->SetRelativeScale3D(FVector(0.24f, 0.24f, 1.22f));
    HeroLeftArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (HeroMeshAsset.Succeeded())
    {
        HeroLeftArmMesh->SetStaticMesh(HeroMeshAsset.Object);
    }
    HeroLeftArmMesh->SetHiddenInGame(bUsingGeneratedHeroMesh);

    HeroRightArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroRightArmMesh"));
    HeroRightArmMesh->SetupAttachment(RootComponent);
    HeroRightArmMesh->SetRelativeLocation(FVector(18.0f, 82.0f, 50.0f));
    HeroRightArmMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 24.0f));
    HeroRightArmMesh->SetRelativeScale3D(FVector(0.26f, 0.26f, 1.32f));
    HeroRightArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (HeroMeshAsset.Succeeded())
    {
        HeroRightArmMesh->SetStaticMesh(HeroMeshAsset.Object);
    }
    HeroRightArmMesh->SetHiddenInGame(bUsingGeneratedHeroMesh);

    HeroHammerHandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroHammerHandleMesh"));
    HeroHammerHandleMesh->SetupAttachment(RootComponent);
    HeroHammerHandleMesh->SetRelativeLocation(FVector(78.0f, 128.0f, 75.0f));
    HeroHammerHandleMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -16.0f));
    HeroHammerHandleMesh->SetRelativeScale3D(FVector(0.13f, 0.13f, 1.55f));
    HeroHammerHandleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (HeroMeshAsset.Succeeded())
    {
        HeroHammerHandleMesh->SetStaticMesh(HeroMeshAsset.Object);
    }

    HeroHammerHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroHammerHeadMesh"));
    HeroHammerHeadMesh->SetupAttachment(RootComponent);
    HeroHammerHeadMesh->SetRelativeLocation(FVector(92.0f, 148.0f, 158.0f));
    HeroHammerHeadMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -16.0f));
    HeroHammerHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PolishedHammerAsset(TEXT("/Game/LightWarrior/Meshes/Weapons/SM_LW_ThunderHammer_Hyper3D.SM_LW_ThunderHammer_Hyper3D"));
    if (PolishedHammerAsset.Succeeded())
    {
        bUsingGeneratedHammerMesh = true;
        HeroHammerHeadMesh->SetStaticMesh(PolishedHammerAsset.Object);
        HeroHammerHeadMesh->SetRelativeLocation(FVector(52.0f, 82.0f, 92.0f));
        HeroHammerHeadMesh->SetRelativeRotation(FRotator(68.0f, -28.0f, -44.0f));
        HeroHammerHeadMesh->SetRelativeScale3D(FVector(0.62f, 0.62f, 0.62f));
        HeroHammerHandleMesh->SetHiddenInGame(true);
    }
    else if (HeroCubeAsset.Succeeded())
    {
        HeroHammerHeadMesh->SetStaticMesh(HeroCubeAsset.Object);
        HeroHammerHeadMesh->SetRelativeScale3D(FVector(0.82f, 0.30f, 0.28f));
    }

    HeroAuraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroAuraMesh"));
    HeroAuraMesh->SetupAttachment(RootComponent);
    HeroAuraMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -78.0f));
    HeroAuraMesh->SetRelativeScale3D(FVector(2.65f, 2.65f, 0.035f));
    HeroAuraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroAuraMesh->SetHiddenInGame(false);

    if (HeroMeshAsset.Succeeded())
    {
        HeroAuraMesh->SetStaticMesh(HeroMeshAsset.Object);
    }

    HeroLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeroLight"));
    HeroLight->SetupAttachment(RootComponent);
    HeroLight->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
    HeroLight->SetLightColor(FLinearColor(0.55f, 0.92f, 1.0f));
    HeroLight->SetIntensity(2200.0f);
    HeroLight->SetAttenuationRadius(720.0f);

    HeroLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HeroLabel"));
    HeroLabel->SetupAttachment(RootComponent);
    HeroLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 235.0f));
    HeroLabel->SetHorizontalAlignment(EHTA_Center);
    HeroLabel->SetTextRenderColor(FColor(235, 255, 255));
    HeroLabel->SetWorldSize(84.0f);
    HeroLabel->SetText(FText::FromString(TEXT("HERO")));
    HeroLabel->SetHiddenInGame(true);

    StrikePulseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StrikePulseMesh"));
    StrikePulseMesh->SetupAttachment(RootComponent);
    StrikePulseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -72.0f));
    StrikePulseMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.025f));
    StrikePulseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StrikePulseMesh->SetHiddenInGame(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> StrikePulseMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (StrikePulseMeshAsset.Succeeded())
    {
        StrikePulseMesh->SetStaticMesh(StrikePulseMeshAsset.Object);
    }

    StrikeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StrikeLight"));
    StrikeLight->SetupAttachment(RootComponent);
    StrikeLight->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
    StrikeLight->SetLightColor(FLinearColor(0.72f, 0.96f, 1.0f));
    StrikeLight->SetIntensity(0.0f);
    StrikeLight->SetAttenuationRadius(1750.0f);

    StrikeLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StrikeLabel"));
    StrikeLabel->SetupAttachment(RootComponent);
    StrikeLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 330.0f));
    StrikeLabel->SetHorizontalAlignment(EHTA_Center);
    StrikeLabel->SetTextRenderColor(FColor(255, 246, 170));
    StrikeLabel->SetWorldSize(104.0f);
    StrikeLabel->SetText(FText::FromString(TEXT("LIGHT STRIKE")));
    StrikeLabel->SetHiddenInGame(true);

    ProgressionComponent = CreateDefaultSubobject<ULightWarriorProgressionComponent>(TEXT("ProgressionComponent"));
}

void ALightWarriorCharacter::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
    TotalDamageTaken = 0.0f;
    DamageEventCount = 0;
    bRecordPlayerInput = FParse::Param(FCommandLine::Get(), TEXT("LWRecordInput"));
    if (bRecordPlayerInput)
    {
        LogRecordedInputAction(TEXT("recording_started"));
    }

    auto ApplyColor = [](UStaticMeshComponent* MeshComponent, const FLinearColor& Color)
    {
        if (!MeshComponent)
        {
            return;
        }

        UMaterialInstanceDynamic* Material = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
        if (Material)
        {
            Material->SetVectorParameterValue(TEXT("Color"), Color);
            Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
        }
    };

    ApplyColor(HeroMesh, FLinearColor(0.18f, 0.74f, 0.95f));
    ApplyColor(HeroHeadMesh, FLinearColor(0.94f, 0.80f, 0.56f));
    ApplyColor(HeroLeftEyeMesh, FLinearColor(0.02f, 0.05f, 0.08f));
    ApplyColor(HeroRightEyeMesh, FLinearColor(0.02f, 0.05f, 0.08f));
    ApplyColor(HeroLeftArmMesh, FLinearColor(0.92f, 0.72f, 0.42f));
    ApplyColor(HeroRightArmMesh, FLinearColor(0.92f, 0.72f, 0.42f));
    ApplyColor(HeroHammerHandleMesh, FLinearColor(0.24f, 0.18f, 0.10f));
    ApplyColor(HeroHammerHeadMesh, FLinearColor(1.0f, 0.78f, 0.18f));
    ApplyColor(HeroAuraMesh, FLinearColor(0.25f, 0.95f, 1.0f));
    ApplyColor(StrikePulseMesh, FLinearColor(0.55f, 0.92f, 1.0f));
}

void ALightWarriorCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    DashCooldownRemaining = FMath::Max(0.0f, DashCooldownRemaining - DeltaSeconds);
    DashTimeRemaining = FMath::Max(0.0f, DashTimeRemaining - DeltaSeconds);
    CheckFallDeath();

    if (IsDefeated())
    {
        return;
    }

    if (DashTimeRemaining <= 0.0f && GetCharacterMovement()->GroundFriction <= DashFriction + KINDA_SMALL_NUMBER)
    {
        FinishDash();
    }

    ThunderHammerTimer = FMath::Max(0.0f, ThunderHammerTimer - DeltaSeconds);
    const float HeroPulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * (HasThunderHammer() ? 10.0f : 4.5f));
    HeroLight->SetIntensity(HasThunderHammer() ? FMath::Lerp(7200.0f, 12000.0f, HeroPulse) : FMath::Lerp(1050.0f, 2400.0f, HeroPulse));
    HeroLight->SetLightColor(HasThunderHammer() ? FLinearColor(1.0f, 0.78f, 0.24f) : FLinearColor(0.55f, 0.92f, 1.0f));
    HeroLabel->SetText(HasThunderHammer() ? FText::FromString(TEXT("THUNDER HERO")) : FText::FromString(TEXT("HERO")));
    HeroLabel->SetTextRenderColor(HasThunderHammer() ? FColor(255, 222, 96) : FColor(235, 255, 255));
    UpdateHeroReadability(DeltaSeconds);
    UpdateStrikePulse(DeltaSeconds);
    ApplyCirclePressure(DeltaSeconds);
}

void ALightWarriorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ALightWarriorCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ALightWarriorCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ALightWarriorCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ALightWarriorCharacter::LookUp);
    PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &ALightWarriorCharacter::StartDash);
    PlayerInputComponent->BindAction(TEXT("LightStrike"), IE_Pressed, this, &ALightWarriorCharacter::LightStrike);
}

float ALightWarriorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (AppliedDamage <= 0.0f || IsDefeated())
    {
        return AppliedDamage;
    }

    const float PreviousHealth = Health;
    Health = FMath::Max(0.0f, Health - AppliedDamage);
    const float ActualDamage = FMath::Max(0.0f, PreviousHealth - Health);
    if (ActualDamage > 0.0f)
    {
        TotalDamageTaken += ActualDamage;
        ++DamageEventCount;
        UE_LOG(LogTemp, Display, TEXT("LW_PLAYER_DAMAGE amount=%.1f health=%.1f events=%d"), ActualDamage, Health, DamageEventCount);
    }
    return AppliedDamage;
}

void ALightWarriorCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
    KillFromFall();
}

void ALightWarriorCharacter::GrantThunderHammer(float DurationSeconds)
{
    ThunderHammerTimer = FMath::Max(ThunderHammerTimer, DurationSeconds);

    if (ProgressionComponent)
    {
        ProgressionComponent->AddResource(ELightWarriorResource::Courage, 20.0f);
        ProgressionComponent->RecordRestoration(25.0f);
    }
}

void ALightWarriorCharacter::TriggerLightStrike()
{
    LightStrike();
}

bool ALightWarriorCharacter::InvestTempleOffering()
{
    if (!ProgressionComponent)
    {
        return false;
    }

    const float LightCost = 45.0f;
    const float ShardCost = 3.0f;
    if (ProgressionComponent->GetResource(ELightWarriorResource::Light) < LightCost
        || ProgressionComponent->GetResource(ELightWarriorResource::ResonanceShards) < ShardCost)
    {
        return false;
    }

    const bool bSpentLight = ProgressionComponent->SpendResource(ELightWarriorResource::Light, LightCost);
    const bool bSpentShards = ProgressionComponent->SpendResource(ELightWarriorResource::ResonanceShards, ShardCost);
    if (!bSpentLight || !bSpentShards)
    {
        return false;
    }

    ProgressionComponent->AddResource(ELightWarriorResource::AetherCharge, 1.0f);
    ProgressionComponent->AddResource(ELightWarriorResource::Courage, 12.0f);
    UE_LOG(LogTemp, Display, TEXT("LW_TEMPLE_OFFERING_SPENT light=%.0f shards=%.0f"), LightCost, ShardCost);
    return true;
}

void ALightWarriorCharacter::MoveForward(float Value)
{
    if (FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator CameraYaw(0.0f, CameraBoom ? CameraBoom->GetComponentRotation().Yaw : GetActorRotation().Yaw, 0.0f);
    AddMovementInput(CameraYaw.Vector(), Value);
    if (bRecordPlayerInput && GetWorld() && GetWorld()->GetTimeSeconds() - LastRecordedMoveInputTime >= 0.35f)
    {
        LastRecordedMoveInputTime = GetWorld()->GetTimeSeconds();
        LogRecordedInputAction(TEXT("move"), FString::Printf(TEXT("axis=forward value=%.2f"), Value));
    }
}

void ALightWarriorCharacter::MoveRight(float Value)
{
    if (FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator CameraYaw(0.0f, CameraBoom ? CameraBoom->GetComponentRotation().Yaw : GetActorRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y), Value);
    if (bRecordPlayerInput && GetWorld() && GetWorld()->GetTimeSeconds() - LastRecordedMoveInputTime >= 0.35f)
    {
        LastRecordedMoveInputTime = GetWorld()->GetTimeSeconds();
        LogRecordedInputAction(TEXT("move"), FString::Printf(TEXT("axis=right value=%.2f"), Value));
    }
}

void ALightWarriorCharacter::Turn(float Value)
{
    if (FMath::IsNearlyZero(Value) || !CameraBoom)
    {
        return;
    }

    const FRotator CurrentRotation = CameraBoom->GetRelativeRotation();
    CameraBoom->SetRelativeRotation(FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw + Value * CameraYawSensitivity, 0.0f));
    if (bRecordPlayerInput && GetWorld() && GetWorld()->GetTimeSeconds() - LastRecordedLookInputTime >= 0.35f)
    {
        LastRecordedLookInputTime = GetWorld()->GetTimeSeconds();
        LogRecordedInputAction(TEXT("look"), FString::Printf(TEXT("axis=yaw value=%.2f"), Value));
    }
}

void ALightWarriorCharacter::LookUp(float Value)
{
    if (FMath::IsNearlyZero(Value) || !CameraBoom)
    {
        return;
    }

    const FRotator CurrentRotation = CameraBoom->GetRelativeRotation();
    const float NewPitch = FMath::Clamp(CurrentRotation.Pitch + Value * CameraPitchSensitivity, MinCameraPitch, MaxCameraPitch);
    CameraBoom->SetRelativeRotation(FRotator(NewPitch, CurrentRotation.Yaw, 0.0f));
    if (bRecordPlayerInput && GetWorld() && GetWorld()->GetTimeSeconds() - LastRecordedLookInputTime >= 0.35f)
    {
        LastRecordedLookInputTime = GetWorld()->GetTimeSeconds();
        LogRecordedInputAction(TEXT("look"), FString::Printf(TEXT("axis=pitch value=%.2f"), Value));
    }
}

void ALightWarriorCharacter::StartDash()
{
    if (DashCooldownRemaining > 0.0f)
    {
        return;
    }

    FVector RawDashDirection = GetLastMovementInputVector();
    if (RawDashDirection.IsNearlyZero())
    {
        RawDashDirection = GetVelocity();
    }
    if (RawDashDirection.IsNearlyZero())
    {
        RawDashDirection = GetActorForwardVector();
    }
    const FVector DashDirection(RawDashDirection.X, RawDashDirection.Y, 0.0f);

    GetCharacterMovement()->GroundFriction = DashFriction;
    GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
    LaunchCharacter(DashDirection.GetSafeNormal() * DashStrength, true, false);
    DashTimeRemaining = DashDuration;
    DashCooldownRemaining = DashCooldown;
    StrikeLight->SetLightColor(FLinearColor(0.55f, 0.92f, 1.0f));
    StrikeLight->SetIntensity(9500.0f);
    StrikeLight->SetAttenuationRadius(950.0f);
    ULightWarriorAudio::PlaySfx(this, ELightWarriorSfx::Dash, GetActorLocation(), 0.85f);
    LogRecordedInputAction(TEXT("dash"));
}

void ALightWarriorCharacter::FinishDash()
{
    GetCharacterMovement()->GroundFriction = NormalGroundFriction;
    GetCharacterMovement()->BrakingDecelerationWalking = NormalBrakingDeceleration;
}

void ALightWarriorCharacter::LightStrike()
{
    const float DamageAmount = HasThunderHammer() ? 80.0f : 35.0f;
    const float Radius = HasThunderHammer() ? StrikeRadius * 1.75f : StrikeRadius;
    bool bHitDarkness = false;
    StrikePulseTimer = StrikePulseDuration;
    StrikePulseMesh->SetHiddenInGame(false);
    StrikeLabel->SetHiddenInGame(false);
    StrikeLabel->SetText(HasThunderHammer() ? FText::FromString(TEXT("THUNDER STRIKE")) : FText::FromString(TEXT("LIGHT STRIKE")));
    StrikeLabel->SetTextRenderColor(HasThunderHammer() ? FColor(255, 210, 72) : FColor(210, 250, 255));
    ULightWarriorAudio::PlaySfx(this, HasThunderHammer() ? ELightWarriorSfx::HammerSlam : ELightWarriorSfx::LightStrike, GetActorLocation(), HasThunderHammer() ? 1.05f : 0.8f);
    LogRecordedInputAction(HasThunderHammer() ? TEXT("thunder_strike") : TEXT("light_strike"));

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LightStrike), false, this);

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
            if (!Enemy)
            {
                continue;
            }

            UGameplayStatics::ApplyDamage(Enemy, DamageAmount, GetController(), this, UDamageType::StaticClass());
            bHitDarkness = true;
        }
    }

    if (bHitDarkness)
    {
        StrikeLabel->SetText(HasThunderHammer() ? FText::FromString(TEXT("SHADOW BROKEN")) : FText::FromString(TEXT("LIGHT EXPANDS")));
        StrikeLabel->SetTextRenderColor(FColor(255, 242, 128));

        if (ASacredCircle* Circle = FindSacredCircle())
        {
            Circle->ExpandFromCombat(HasThunderHammer() ? StrikeExpansionAmount * 2.4f : StrikeExpansionAmount);
        }

        if (ProgressionComponent)
        {
            ProgressionComponent->RecordRestoration(HasThunderHammer() ? 18.0f : 8.0f);
        }
    }
}

void ALightWarriorCharacter::UpdateStrikePulse(float DeltaSeconds)
{
    StrikePulseTimer = FMath::Max(0.0f, StrikePulseTimer - DeltaSeconds);
    if (StrikePulseTimer <= 0.0f)
    {
        StrikePulseMesh->SetHiddenInGame(true);
        StrikeLabel->SetHiddenInGame(true);
        StrikeLight->SetIntensity(0.0f);
        return;
    }

    const float Alpha = 1.0f - (StrikePulseTimer / StrikePulseDuration);
    const float PulseRadius = HasThunderHammer()
        ? FMath::Lerp(2.0f, 15.0f, Alpha)
        : FMath::Lerp(1.4f, 9.0f, Alpha);
    const float Flash = FMath::Square(1.0f - Alpha);

    StrikePulseMesh->SetRelativeScale3D(FVector(PulseRadius, PulseRadius, 0.025f));
    StrikeLight->SetIntensity(HasThunderHammer() ? 26000.0f * Flash : 15000.0f * Flash);
    StrikeLight->SetLightColor(HasThunderHammer() ? FLinearColor(1.0f, 0.74f, 0.18f) : FLinearColor(0.72f, 0.96f, 1.0f));
    StrikeLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 330.0f + Alpha * 95.0f));
}

void ALightWarriorCharacter::UpdateHeroReadability(float DeltaSeconds)
{
    const float SpeedAlpha = FMath::Clamp(GetVelocity().Size2D() / FMath::Max(1.0f, GetCharacterMovement()->MaxWalkSpeed), 0.0f, 1.0f);
    const float DashAlpha = DashTimeRemaining > 0.0f ? 1.0f : 0.0f;
    const float ThunderAlpha = HasThunderHammer() ? 1.0f : 0.0f;
    const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * (HasThunderHammer() ? 13.0f : 7.0f));

    if (HeroAuraMesh)
    {
        const float AuraScale = 2.45f + SpeedAlpha * 0.42f + DashAlpha * 1.45f + ThunderAlpha * 0.95f + Pulse * 0.22f;
        HeroAuraMesh->SetRelativeScale3D(FVector(AuraScale, AuraScale, 0.035f));
    }

    if (HeroHammerHeadMesh)
    {
        const float HammerGlowScale = 1.0f + ThunderAlpha * 0.28f + DashAlpha * 0.12f + Pulse * ThunderAlpha * 0.16f;
        const FVector BaseHammerScale = bUsingGeneratedHammerMesh
            ? FVector(0.62f, 0.62f, 0.62f)
            : FVector(0.82f, 0.30f, 0.28f);
        HeroHammerHeadMesh->SetRelativeScale3D(BaseHammerScale * HammerGlowScale);
        if (bUsingGeneratedHammerMesh)
        {
            const float HammerLift = ThunderAlpha * 30.0f + DashAlpha * 12.0f + Pulse * ThunderAlpha * 8.0f;
            HeroHammerHeadMesh->SetRelativeLocation(FVector(52.0f, 82.0f, 92.0f + HammerLift));
            HeroHammerHeadMesh->SetRelativeRotation(FRotator(68.0f - ThunderAlpha * 24.0f, -28.0f, -44.0f + DashAlpha * 12.0f));
        }
    }

    if (!bUsingGeneratedHeroMesh && HeroLeftArmMesh && HeroRightArmMesh)
    {
        const float ArmSwing = FMath::Sin(GetWorld()->GetTimeSeconds() * 10.0f) * SpeedAlpha * 7.0f;
        HeroLeftArmMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -28.0f - ArmSwing));
        HeroRightArmMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 24.0f + ArmSwing));
    }
}

void ALightWarriorCharacter::LogRecordedInputAction(const TCHAR* ActionName, const FString& Details) const
{
    if (!bRecordPlayerInput)
    {
        return;
    }

    const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const FVector Location = GetActorLocation();
    if (Details.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_PLAYER_INPUT_ACTION %s time=%.2f location=V(X=%.2f, Y=%.2f, Z=%.2f)"),
            ActionName,
            TimeSeconds,
            Location.X,
            Location.Y,
            Location.Z);
        return;
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("LW_PLAYER_INPUT_ACTION %s time=%.2f location=V(X=%.2f, Y=%.2f, Z=%.2f) %s"),
        ActionName,
        TimeSeconds,
        Location.X,
        Location.Y,
        Location.Z,
        *Details);
}

void ALightWarriorCharacter::CheckFallDeath()
{
    if (!bKilledByFall && GetActorLocation().Z <= FallDeathZ)
    {
        KillFromFall();
    }
}

void ALightWarriorCharacter::KillFromFall()
{
    if (bKilledByFall)
    {
        return;
    }

    bKilledByFall = true;
    Health = 0.0f;
    ThunderHammerTimer = 0.0f;
    DashTimeRemaining = 0.0f;
    FinishDash();
}

void ALightWarriorCharacter::ApplyCirclePressure(float DeltaSeconds)
{
    const ASacredCircle* Circle = FindSacredCircle();
    if (!Circle)
    {
        return;
    }

    const bool bInsideCircle = Circle->IsInsideCircle(GetActorLocation());
    if (bInsideCircle)
    {
        bWasInsideCircle = true;
        bAwardedCurrentExcursion = false;
        return;
    }

    if (bWasInsideCircle && !bAwardedCurrentExcursion && ProgressionComponent)
    {
        ProgressionComponent->RecordBraveExcursion();
        bAwardedCurrentExcursion = true;
    }

    bWasInsideCircle = false;
    Health = FMath::Max(0.0f, Health - OutsideCircleDamagePerSecond * DeltaSeconds);
}

ASacredCircle* ALightWarriorCharacter::FindSacredCircle() const
{
    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        return *It;
    }

    return nullptr;
}
