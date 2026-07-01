#include "LightWarriorGameMode.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "LightWell.h"
#include "LightShardPickup.h"
#include "LightWarriorCharacter.h"
#include "LightWarriorProgressionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorHUD.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "SacredCircle.h"
#include "ShadowEnemy.h"
#include "ThunderHammerTemple.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
void ApplyMeshColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color, bool bEmissive)
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

ALightWarriorGameMode::ALightWarriorGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = ALightWarriorCharacter::StaticClass();
    HUDClass = ALightWarriorHUD::StaticClass();
    RemainingRunSeconds = RunDurationSeconds;
}

void ALightWarriorGameMode::BeginPlay()
{
    Super::BeginPlay();

    BootstrapPlayableArenaIfNeeded();
    BindLightWellObjectives();
    RemainingRunSeconds = RunDurationSeconds;
    if (bAutoStartRun)
    {
        StartRun();
    }

    ConfigureAutomationLoop();
}

void ALightWarriorGameMode::RestartPlayer(AController* NewPlayer)
{
    if (!NewPlayer || NewPlayer->GetPawn())
    {
        return;
    }

    const FTransform SpawnTransform(FRotator(0.0f, 90.0f, 0.0f), FVector(-420.0f, 0.0f, 140.0f));
    if (APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform))
    {
        NewPlayer->SetPawn(NewPawn);
        NewPlayer->Possess(NewPawn);
        return;
    }

    Super::RestartPlayer(NewPlayer);
}

void ALightWarriorGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (RunState != ELightWarriorRunState::Running)
    {
        return;
    }

    if (IsPlayerDefeated())
    {
        bAutomationRouteActive = false;
        CompleteRun(false);
        return;
    }

    UpdateAutomationRoute(DeltaSeconds);
    TryStartFinalWave();
    UpdateFinalWave(DeltaSeconds);

    RemainingRunSeconds = FMath::Max(0.0f, RemainingRunSeconds - DeltaSeconds);
    OnRunTimeChanged.Broadcast(RemainingRunSeconds);

    if (RemainingRunSeconds <= 0.0f)
    {
        CompleteRun(false);
    }
}

void ALightWarriorGameMode::StartRun()
{
    PurifiedLightWells = 0;
    bTempleUnlocked = false;
    bFinalWaveStarted = false;
    bFinalWaveCleared = false;
    FinalWaveRemainingSeconds = 0.0f;
    ActiveFinalWaveEnemies.Empty();
    AutomationRouteSteps.Empty();
    AutomationRouteStepIndex = INDEX_NONE;
    AutomationRouteHoldRemaining = 0.0f;
    AutomationRouteStartWorldSeconds = 0.0f;
    AutomationCombatStrikeCooldownRemaining = 0.0f;
    AutomationInputHoldSeconds = 0.0f;
    AutomationInputMoveSamples = 0;
    AutomationInputCombatActions = 0;
    AutomationInputOfferingActions = 0;
    AutomationInputSurvivalActions = 0;
    AutomationCombatGatesStarted = 0;
    AutomationCombatGatesCompleted = 0;
    ActiveAutomationCombatGateEnemies.Empty();
    bAutomationRouteActive = false;
    bAutomationRouteStepActionFired = false;
    bAutomationRouteStepMoveLogged = false;
    bAutomationCombatGateActive = false;
    bAutomationSurvivalPressureApplied = false;
    if (AThunderHammerTemple* Temple = FindThunderHammerTemple())
    {
        Temple->SetTempleUnlocked(false);
    }
    RemainingRunSeconds = RunDurationSeconds;
    OnObjectiveProgressChanged.Broadcast(PurifiedLightWells, RequiredLightWells);
    OnRunTimeChanged.Broadcast(RemainingRunSeconds);
    SetRunState(ELightWarriorRunState::Running);
}

void ALightWarriorGameMode::CompleteRun(bool bVictory)
{
    SetRunState(bVictory ? ELightWarriorRunState::Victory : ELightWarriorRunState::Failure);
    if (AutomationScenario.Equals(TEXT("first-temple-run-route-proof"), ESearchCase::IgnoreCase)
        || AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase))
    {
        const float RouteElapsedSeconds = GetWorld()
            ? FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - AutomationRouteStartWorldSeconds)
            : 0.0f;
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_ROUTE_PROOF_%s elapsed=%.2f remaining=%.2f"),
            bVictory ? TEXT("COMPLETE") : TEXT("FAILED"),
            RouteElapsedSeconds,
            RemainingRunSeconds);
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_ROUTE_COMBAT_GATES started=%d completed=%d"),
            AutomationCombatGatesStarted,
            AutomationCombatGatesCompleted);
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_ROUTE_INPUT move_samples=%d hold_seconds=%.1f combat_actions=%d offering_actions=%d survival_actions=%d"),
            AutomationInputMoveSamples,
            AutomationInputHoldSeconds,
            AutomationInputCombatActions,
            AutomationInputOfferingActions,
            AutomationInputSurvivalActions);
        if (const ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
        {
            if (const ULightWarriorProgressionComponent* Progression = Character->GetProgressionComponent())
            {
                UE_LOG(
                    LogTemp,
                    Display,
                    TEXT("LW_ROUTE_RESOURCES light=%.0f courage=%.0f shards=%.0f aether=%.0f"),
                    Progression->GetResource(ELightWarriorResource::Light),
                    Progression->GetResource(ELightWarriorResource::Courage),
                    Progression->GetResource(ELightWarriorResource::ResonanceShards),
                    Progression->GetResource(ELightWarriorResource::AetherCharge));
            }
            UE_LOG(
                LogTemp,
                Display,
                TEXT("LW_ROUTE_SURVIVAL health=%.0f max=%.0f damage=%.0f damage_events=%d pressure_applied=%d"),
                Character->GetHealth(),
                Character->GetMaxHealth(),
                Character->GetTotalDamageTaken(),
                Character->GetDamageEventCount(),
                bAutomationSurvivalPressureApplied ? 1 : 0);
        }
    }

    if (!bVictory && bRestartAfterFailure && GetWorld())
    {
        FTimerHandle RestartTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            RestartTimerHandle,
            this,
            &ALightWarriorGameMode::RestartLevelAfterFailure,
            FailureRestartDelaySeconds,
            false);
    }
}

void ALightWarriorGameMode::ConfigureAutomationLoop()
{
    const TCHAR* CommandLine = FCommandLine::Get();
    const bool bAutoCapture = FParse::Param(CommandLine, TEXT("LWAutoCapture"));
    if (!bAutoCapture || !GetWorld())
    {
        return;
    }

    float ShotDelaySeconds = 5.0f;
    FParse::Value(CommandLine, TEXT("LWShotDelay="), ShotDelaySeconds);

    float QuitDelaySeconds = 1.5f;
    FParse::Value(CommandLine, TEXT("LWQuitDelay="), QuitDelaySeconds);

    if (!FParse::Value(CommandLine, TEXT("LWShot="), AutomationScreenshotPath) || AutomationScreenshotPath.IsEmpty())
    {
        AutomationScreenshotPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots"), TEXT("LightWarriorAutomation.png"));
    }
    FParse::Value(CommandLine, TEXT("LWScenario="), AutomationScenario);

    const FString ScreenshotDirectory = FPaths::GetPath(AutomationScreenshotPath);
    if (!ScreenshotDirectory.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(*ScreenshotDirectory, true);
    }

    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
    {
        PlayerController->ConsoleCommand(TEXT("DisableAllScreenMessages"), true);
    }

    if (AutomationScenario.Equals(TEXT("first-light-well-loop"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::PlaceAutomationPlayerAtFirstLightWell,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("combat-readability"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::PlaceAutomationPlayerInCombat,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("thunder-hammer"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::PlaceAutomationPlayerAtThunderHammer,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("first-temple-run"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::PlaceAutomationPlayerAtFirstTempleRun,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("first-temple-run-proof"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::StartAutomationFirstTempleRunProof,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("first-temple-run-route-proof"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::StartAutomationFirstTempleRunRouteProof,
            0.75f,
            false);
    }
    else if (AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase))
    {
        FTimerHandle ScenarioTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            ScenarioTimerHandle,
            this,
            &ALightWarriorGameMode::StartAutomationFirstTempleRunPacedProof,
            0.75f,
            false);
    }

    FTimerHandle CaptureTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        CaptureTimerHandle,
        this,
        &ALightWarriorGameMode::CaptureAutomationScreenshot,
        FMath::Max(0.25f, ShotDelaySeconds),
        false);

    FTimerHandle QuitTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        QuitTimerHandle,
        this,
        &ALightWarriorGameMode::QuitAfterAutomationCapture,
        FMath::Max(0.5f, ShotDelaySeconds + QuitDelaySeconds),
        false);
}

void ALightWarriorGameMode::CaptureAutomationScreenshot()
{
    if (!AutomationScreenshotPath.IsEmpty())
    {
        FScreenshotRequest::RequestScreenshot(AutomationScreenshotPath, true, false);
    }
}

void ALightWarriorGameMode::QuitAfterAutomationCapture()
{
    FPlatformMisc::RequestExit(false, TEXT("LightWarrior automation capture complete"));
}

void ALightWarriorGameMode::BootstrapPlayableArenaIfNeeded()
{
    if (!bAutoSpawnArenaIfEmpty)
    {
        return;
    }

    bool bHasCircle = false;
    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        bHasCircle = true;
        break;
    }

    if (bHasCircle)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -60.0f), FRotator::ZeroRotator, SpawnParams);
    if (Floor)
    {
        if (UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
        {
            Floor->GetStaticMeshComponent()->SetStaticMesh(FloorMesh);
        }
        Floor->SetActorScale3D(FVector(420.0f, 420.0f, 0.08f));
        Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        ApplyMeshColor(Floor->GetStaticMeshComponent(), FLinearColor(0.055f, 0.068f, 0.074f), false);
        Floor->SetActorLabel(TEXT("LW_ProceduralArenaFloor"));
    }

    ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(-1400.0f, -900.0f, 1800.0f), FRotator(-48.0f, -35.0f, 0.0f), SpawnParams);
    if (Sun)
    {
        Sun->SetActorLabel(TEXT("LW_ProceduralSun"));
    }

    ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(FVector(0.0f, 0.0f, 900.0f), FRotator::ZeroRotator, SpawnParams);
    if (SkyLight)
    {
        SkyLight->SetActorLabel(TEXT("LW_ProceduralSkyLight"));
    }

    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
    if (Fog)
    {
        Fog->SetActorLabel(TEXT("LW_ProceduralAetherFog"));
    }

    APostProcessVolume* ExposureVolume = World->SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (ExposureVolume)
    {
        ExposureVolume->SetActorLabel(TEXT("LW_ProceduralExposure"));
        ExposureVolume->bUnbound = true;
        ExposureVolume->Priority = 10.0f;
        ExposureVolume->BlendWeight = 1.0f;
        ExposureVolume->Settings.bOverride_AutoExposureMethod = true;
        ExposureVolume->Settings.AutoExposureMethod = AEM_Manual;
        ExposureVolume->Settings.bOverride_AutoExposureMinBrightness = true;
        ExposureVolume->Settings.bOverride_AutoExposureMaxBrightness = true;
        ExposureVolume->Settings.AutoExposureMinBrightness = 0.0f;
        ExposureVolume->Settings.AutoExposureMaxBrightness = 0.0f;
        ExposureVolume->Settings.bOverride_AutoExposureBias = true;
        ExposureVolume->Settings.AutoExposureBias = -1.28f;
        ExposureVolume->Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
        ExposureVolume->Settings.AutoExposureApplyPhysicalCameraExposure = false;
    }

    ASacredCircle* Circle = World->SpawnActor<ASacredCircle>(ASacredCircle::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (Circle)
    {
        Circle->SetActorLabel(TEXT("LW_SacredCircle"));
    }

    const FVector LightWellLocations[] =
    {
        FVector(1450.0f, 0.0f, 0.0f),
        FVector(-780.0f, 1320.0f, 0.0f),
        FVector(-780.0f, -1320.0f, 0.0f)
    };

    if (UStaticMesh* PathMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        for (const FVector& LightWellLocation : LightWellLocations)
        {
            const FVector Midpoint = LightWellLocation * 0.5f + FVector(0.0f, 0.0f, -36.0f);
            const float Length = LightWellLocation.Size2D();
            const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(LightWellLocation.Y, LightWellLocation.X));
            AStaticMeshActor* Path = World->SpawnActor<AStaticMeshActor>(Midpoint, FRotator(0.0f, Yaw, 0.0f), SpawnParams);
            if (Path)
            {
                Path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
                Path->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                Path->SetActorScale3D(FVector(Length / 100.0f, 1.25f, 0.035f));
                ApplyMeshColor(Path->GetStaticMeshComponent(), FLinearColor(0.055f, 0.30f, 0.34f), false);
                Path->SetActorLabel(TEXT("LW_LightPath"));
            }

            APointLight* PathLight = World->SpawnActor<APointLight>(Midpoint + FVector(0.0f, 0.0f, 190.0f), FRotator::ZeroRotator, SpawnParams);
            if (PathLight && PathLight->PointLightComponent)
            {
                PathLight->PointLightComponent->SetLightColor(FLinearColor(0.38f, 0.94f, 1.0f));
                PathLight->PointLightComponent->SetIntensity(760.0f);
                PathLight->PointLightComponent->SetAttenuationRadius(520.0f);
                PathLight->SetActorLabel(TEXT("LW_AetherPathLight"));
            }
        }
    }

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(LightWellLocations); ++Index)
    {
        ALightWell* LightWell = World->SpawnActor<ALightWell>(ALightWell::StaticClass(), LightWellLocations[Index], FRotator::ZeroRotator, SpawnParams);
        if (LightWell)
        {
            LightWell->SetActorLabel(FString::Printf(TEXT("LW_LightWell_%02d"), Index + 1));
        }

        const FVector TowardOrigin = (-FVector(LightWellLocations[Index].X, LightWellLocations[Index].Y, 0.0f)).GetSafeNormal();
        const FVector Tangent(-TowardOrigin.Y, TowardOrigin.X, 0.0f);
        SpawnLightShard(LightWellLocations[Index] + TowardOrigin * 360.0f + Tangent * 160.0f + FVector(0.0f, 0.0f, 70.0f), TEXT("LW_WellRouteShard"), Index * 2 + 1);
        SpawnLightShard(LightWellLocations[Index] + TowardOrigin * 640.0f - Tangent * 180.0f + FVector(0.0f, 0.0f, 70.0f), TEXT("LW_WellRouteShard"), Index * 2 + 2);
    }

    SpawnPressureEnemy(FVector(910.0f, -260.0f, 90.0f), TEXT("LW_FirstWellSentinel"), 1, EShadowEnemyArchetype::ShadowImp);
    SpawnPressureEnemy(FVector(1030.0f, 300.0f, 90.0f), TEXT("LW_FirstWellSentinel"), 2, EShadowEnemyArchetype::Berserker);

    AThunderHammerTemple* Temple = World->SpawnActor<AThunderHammerTemple>(AThunderHammerTemple::StaticClass(), FVector(0.0f, 2200.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
    if (Temple)
    {
        Temple->SetActorLabel(TEXT("LW_ThunderHammerTemple"));
        Temple->SetTempleUnlocked(false);
    }

    SpawnLightShard(FVector(-180.0f, 1240.0f, 70.0f), TEXT("LW_TempleRouteShard"), 1);
    SpawnLightShard(FVector(180.0f, 1640.0f, 70.0f), TEXT("LW_TempleRouteShard"), 2);

    // Corruption crystal clusters around the arena edge
    if (UStaticMesh* CrystalMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Arena/SM_LW_CorruptionCrystals.SM_LW_CorruptionCrystals")))
    {
        const float CrystalRingRadius = 3200.0f;
        for (int32 CrystalIndex = 0; CrystalIndex < 8; ++CrystalIndex)
        {
            const float CrystalAngle = (2.0f * PI * CrystalIndex) / 8.0f + 0.4f;
            const FVector CrystalLocation(
                FMath::Cos(CrystalAngle) * CrystalRingRadius,
                FMath::Sin(CrystalAngle) * CrystalRingRadius,
                0.0f);
            AStaticMeshActor* Crystal = World->SpawnActor<AStaticMeshActor>(CrystalLocation, FRotator(0.0f, CrystalAngle * 180.0f / PI, 0.0f), SpawnParams);
            if (Crystal)
            {
                Crystal->GetStaticMeshComponent()->SetStaticMesh(CrystalMesh);
                Crystal->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                Crystal->SetActorScale3D(FVector(100.0f + CrystalIndex * 15.0f));
                Crystal->SetActorLabel(FString::Printf(TEXT("LW_CorruptionCrystal_%02d"), CrystalIndex));
            }
        }
    }

    // Temple pillars flanking light paths
    if (UStaticMesh* PillarMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/LightWarrior/Meshes/Arena/SM_LW_TemplePillar.SM_LW_TemplePillar")))
    {
        const FVector PillarOffsets[] = {
            FVector(700.0f, -200.0f, 0.0f),
            FVector(700.0f, 200.0f, 0.0f),
            FVector(-350.0f, 700.0f, 0.0f),
            FVector(-350.0f, -700.0f, 0.0f),
        };
        for (int32 PillarIndex = 0; PillarIndex < UE_ARRAY_COUNT(PillarOffsets); ++PillarIndex)
        {
            AStaticMeshActor* Pillar = World->SpawnActor<AStaticMeshActor>(PillarOffsets[PillarIndex], FRotator::ZeroRotator, SpawnParams);
            if (Pillar)
            {
                Pillar->GetStaticMeshComponent()->SetStaticMesh(PillarMesh);
                Pillar->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                Pillar->SetActorScale3D(FVector(150.0f));
                Pillar->SetActorLabel(FString::Printf(TEXT("LW_TemplePillar_%02d"), PillarIndex));
            }
        }
    }

    const float EnemyRingRadius = 2300.0f;
    for (int32 Index = 0; Index < AutoSpawnEnemyCount; ++Index)
    {
        const float AngleRadians = (2.0f * PI * Index) / FMath::Max(1, AutoSpawnEnemyCount);
        const FVector EnemyLocation(
            FMath::Cos(AngleRadians) * EnemyRingRadius,
            FMath::Sin(AngleRadians) * EnemyRingRadius,
            0.0f);

        AShadowEnemy* Enemy = World->SpawnActor<AShadowEnemy>(AShadowEnemy::StaticClass(), EnemyLocation, FRotator::ZeroRotator, SpawnParams);
        if (Enemy)
        {
            Enemy->ConfigureArchetype(Index % 3 == 0 ? EShadowEnemyArchetype::Berserker : EShadowEnemyArchetype::ShadowImp);
            Enemy->SetActorLabel(FString::Printf(TEXT("LW_ShadowEnemy_%02d"), Index + 1));
        }
    }
}

bool ALightWarriorGameMode::IsPlayerDefeated() const
{
    const ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    return Character && Character->IsDefeated();
}

int32 ALightWarriorGameMode::GetActiveFinalWaveEnemies() const
{
    int32 ActiveCount = 0;
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : ActiveFinalWaveEnemies)
    {
        if (Enemy.IsValid())
        {
            ++ActiveCount;
        }
    }
    return ActiveCount;
}

void ALightWarriorGameMode::RestartLevelAfterFailure()
{
    if (GetWorld() && RunState == ELightWarriorRunState::Failure)
    {
        const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
    }
}

void ALightWarriorGameMode::BindLightWellObjectives()
{
    int32 DiscoveredLightWells = 0;
    for (TActorIterator<ALightWell> It(GetWorld()); It; ++It)
    {
        ++DiscoveredLightWells;
        It->OnPurificationStarted.AddDynamic(this, &ALightWarriorGameMode::HandleLightWellPurificationStarted);
        It->OnPurified.AddDynamic(this, &ALightWarriorGameMode::HandleLightWellPurified);
    }

    if (DiscoveredLightWells > 0)
    {
        RequiredLightWells = FMath::Min(RequiredLightWells, DiscoveredLightWells);
    }

    RequiredLightWells = FMath::Max(1, RequiredLightWells);
    OnObjectiveProgressChanged.Broadcast(PurifiedLightWells, RequiredLightWells);
}

void ALightWarriorGameMode::PlaceAutomationPlayerAtFirstLightWell()
{
    UWorld* World = GetWorld();
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!World || !Character)
    {
        return;
    }

    ALightWell* FirstWell = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    for (TActorIterator<ALightWell> It(World); It; ++It)
    {
        const float DistanceSquared = It->GetActorLocation().SizeSquared2D();
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            FirstWell = *It;
        }
    }

    if (FirstWell)
    {
        Character->SetActorLocation(FirstWell->GetActorLocation() + FVector(-120.0f, 0.0f, 140.0f), false, nullptr, ETeleportType::TeleportPhysics);
        Character->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
}

void ALightWarriorGameMode::PlaceAutomationPlayerInCombat()
{
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Character)
    {
        return;
    }

    const FVector PlayerLocation(760.0f, -360.0f, 140.0f);
    Character->SetActorLocation(PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
    Character->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    SpawnPressureEnemy(PlayerLocation + FVector(150.0f, 0.0f, -45.0f), TEXT("LW_CombatImp"), 1, EShadowEnemyArchetype::ShadowImp);
    SpawnPressureEnemy(PlayerLocation + FVector(420.0f, 260.0f, -45.0f), TEXT("LW_CombatBerserker"), 1, EShadowEnemyArchetype::Berserker);
}

void ALightWarriorGameMode::PlaceAutomationPlayerAtThunderHammer()
{
    StageAutomationThunderHammer(true);
}

void ALightWarriorGameMode::StageAutomationThunderHammer(bool bSpawnShowcaseEnemies)
{
    UWorld* World = GetWorld();
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!World || !Character)
    {
        return;
    }

    AThunderHammerTemple* Temple = FindThunderHammerTemple();

    if (!Temple)
    {
        return;
    }

    const FVector TempleLocation = Temple->GetActorLocation();
    Temple->SetTempleUnlocked(true);
    Character->SetActorLocation(TempleLocation + FVector(-120.0f, -80.0f, 140.0f), false, nullptr, ETeleportType::TeleportPhysics);
    Character->SetActorRotation(FRotator(0.0f, 45.0f, 0.0f));
    Temple->ActivateTemple(Character);

    if (bSpawnShowcaseEnemies)
    {
        SpawnPressureEnemy(TempleLocation + FVector(520.0f, 90.0f, 90.0f), TEXT("LW_HammerImp"), 1, EShadowEnemyArchetype::ShadowImp);
        SpawnPressureEnemy(TempleLocation + FVector(-520.0f, -160.0f, 90.0f), TEXT("LW_HammerBerserker"), 1, EShadowEnemyArchetype::Berserker);
    }
}

void ALightWarriorGameMode::PlaceAutomationPlayerAtFirstTempleRun()
{
    PurifiedLightWells = RequiredLightWells;
    bTempleUnlocked = true;
    OnObjectiveProgressChanged.Broadcast(PurifiedLightWells, RequiredLightWells);
    SpawnTempleUnlockRewards();
    StageAutomationThunderHammer(false);
    if (ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        Character->SetActorLocation(GetFinalWaveCenter() + FVector(-160.0f, -220.0f, 140.0f), false, nullptr, ETeleportType::TeleportPhysics);
        Character->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
    TryStartFinalWave();
}

void ALightWarriorGameMode::StartAutomationFirstTempleRunProof()
{
    UWorld* World = GetWorld();
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!World || !Character)
    {
        return;
    }

    const FVector WellTargets[] =
    {
        FVector(1450.0f, 0.0f, 0.0f),
        FVector(-780.0f, 1320.0f, 0.0f),
        FVector(-780.0f, -1320.0f, 0.0f)
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(WellTargets); ++Index)
    {
        ALightWell* LightWell = FindAutomationLightWellNear(WellTargets[Index]);
        if (!LightWell)
        {
            continue;
        }

        FTimerHandle MoveToWellTimerHandle;
        FTimerDelegate MoveToWellDelegate;
        const FVector HoldLocation = LightWell->GetActorLocation() + FVector(0.0f, 0.0f, 140.0f);
        const float Yaw = Index == 0 ? 0.0f : (Index == 1 ? -42.0f : 42.0f);
        MoveToWellDelegate.BindLambda([this, HoldLocation, Yaw]()
        {
            MoveAutomationPlayerToLocation(HoldLocation, Yaw);
        });
        World->GetTimerManager().SetTimer(MoveToWellTimerHandle, MoveToWellDelegate, 0.2f + Index * 5.15f, false);
    }

    FTimerHandle TempleTimerHandle;
    FTimerDelegate TempleDelegate;
    TempleDelegate.BindLambda([this]()
    {
        if (AThunderHammerTemple* Temple = FindThunderHammerTemple())
        {
            const FVector TempleLocation = Temple->GetActorLocation();
            MoveAutomationPlayerToLocation(TempleLocation + FVector(-90.0f, -80.0f, 140.0f), 45.0f);
            Temple->ActivateTemple(Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)));
        }
    });
    World->GetTimerManager().SetTimer(TempleTimerHandle, TempleDelegate, 16.35f, false);

    FTimerHandle FinalWaveTimerHandle;
    FTimerDelegate FinalWaveDelegate;
    FinalWaveDelegate.BindLambda([this]()
    {
        MoveAutomationPlayerToLocation(GetFinalWaveCenter() + FVector(-160.0f, -220.0f, 140.0f), 0.0f);
        TryStartFinalWave();
    });
    World->GetTimerManager().SetTimer(FinalWaveTimerHandle, FinalWaveDelegate, 17.25f, false);

    FTimerHandle FinalStrikeTimerHandle;
    World->GetTimerManager().SetTimer(
        FinalStrikeTimerHandle,
        this,
        &ALightWarriorGameMode::BreakAutomationFinalWaveEnemies,
        26.0f,
        false);
}

void ALightWarriorGameMode::StartAutomationFirstTempleRunRouteProof()
{
    StartAutomationFirstTempleRunPacedProof();
}

void ALightWarriorGameMode::StartAutomationFirstTempleRunPacedProof()
{
    AutomationRouteSteps.Empty();
    AutomationRouteStepIndex = 0;
    AutomationRouteHoldRemaining = 0.0f;
    AutomationCombatStrikeCooldownRemaining = 0.0f;
    AutomationInputHoldSeconds = 0.0f;
    AutomationInputMoveSamples = 0;
    AutomationInputCombatActions = 0;
    AutomationInputOfferingActions = 0;
    AutomationInputSurvivalActions = 0;
    AutomationCombatGatesStarted = 0;
    AutomationCombatGatesCompleted = 0;
    ActiveAutomationCombatGateEnemies.Empty();
    bAutomationRouteStepActionFired = false;
    bAutomationRouteStepMoveLogged = false;
    bAutomationCombatGateActive = false;
    bAutomationSurvivalPressureApplied = false;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        FTimerHandle RetryTimerHandle;
        World->GetTimerManager().SetTimer(
            RetryTimerHandle,
            this,
            AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase)
                ? &ALightWarriorGameMode::StartAutomationFirstTempleRunPacedProof
                : &ALightWarriorGameMode::StartAutomationFirstTempleRunRouteProof,
            0.35f,
            false);
        return;
    }

    const bool bPacedProof = AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase);
    const float WellHoldSeconds = bPacedProof ? 24.0f : 4.7f;
    const float TempleHoldSeconds = bPacedProof ? 15.0f : 0.8f;
    const float FinalWaveHoldSeconds = bPacedProof ? 70.0f : 8.2f;

    auto AddRouteStep = [this](
        const FVector& Target,
        float AcceptanceRadius,
        float HoldSeconds,
        bool bActivateTemple,
        bool bStartFinalWave,
        bool bBreakFinalWave,
        int32 GateImpCount = 0,
        int32 GateBerserkerCount = 0)
    {
        FAutomationRouteStep Step;
        Step.Target = Target;
        Step.AcceptanceRadius = AcceptanceRadius;
        Step.HoldSeconds = HoldSeconds;
        Step.bActivateTemple = bActivateTemple;
        Step.bStartFinalWave = bStartFinalWave;
        Step.bBreakFinalWave = bBreakFinalWave;
        Step.GateImpCount = GateImpCount;
        Step.GateBerserkerCount = GateBerserkerCount;
        AutomationRouteSteps.Add(Step);
    };

    AddRouteStep(FVector(1450.0f, 0.0f, 140.0f), 135.0f, WellHoldSeconds, false, false, false, bPacedProof ? 2 : 0, 0);
    AddRouteStep(FVector(-780.0f, 1320.0f, 140.0f), 150.0f, WellHoldSeconds, false, false, false, bPacedProof ? 2 : 0, 0);
    AddRouteStep(FVector(-780.0f, -1320.0f, 140.0f), 150.0f, WellHoldSeconds, false, false, false, bPacedProof ? 3 : 0, 0);

    if (AThunderHammerTemple* Temple = FindThunderHammerTemple())
    {
        const FVector TempleLocation = Temple->GetActorLocation();
        AddRouteStep(TempleLocation + FVector(-95.0f, -80.0f, 140.0f), 150.0f, TempleHoldSeconds, true, false, false);
    }
    else
    {
        AddRouteStep(FVector(-95.0f, 2120.0f, 140.0f), 150.0f, TempleHoldSeconds, true, false, false);
    }

    AddRouteStep(GetFinalWaveCenter() + FVector(-160.0f, -220.0f, 140.0f), 150.0f, FinalWaveHoldSeconds, false, true, false);
    AddRouteStep(GetFinalWaveCenter() + FVector(-160.0f, -220.0f, 140.0f), 180.0f, 0.2f, false, false, true);

    bAutomationRouteActive = AutomationRouteSteps.Num() > 0;
    if (bAutomationRouteActive)
    {
        AutomationRouteStartWorldSeconds = World->GetTimeSeconds();
        UE_LOG(
            LogTemp,
            Display,
            TEXT("LW_ROUTE_PROOF_STARTED steps=%d target_seconds=180.00 paced=%d"),
            AutomationRouteSteps.Num(),
            bPacedProof ? 1 : 0);
    }
}

void ALightWarriorGameMode::UpdateAutomationRoute(float DeltaSeconds)
{
    if (!bAutomationRouteActive || RunState != ELightWarriorRunState::Running)
    {
        return;
    }

    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Character || !AutomationRouteSteps.IsValidIndex(AutomationRouteStepIndex))
    {
        bAutomationRouteActive = false;
        return;
    }

    FAutomationRouteStep& Step = AutomationRouteSteps[AutomationRouteStepIndex];
    const FVector CharacterLocation = Character->GetActorLocation();
    const FVector ToTarget = Step.Target - CharacterLocation;
    const float Distance2D = ToTarget.Size2D();

    if (Distance2D > Step.AcceptanceRadius)
    {
        const FVector Direction(ToTarget.X, ToTarget.Y, 0.0f);
        const FVector MoveDirection = Direction.GetSafeNormal();
        if (!bAutomationRouteStepMoveLogged)
        {
            UE_LOG(LogTemp, Display, TEXT("LW_INPUT_ACTION move step=%d target=%s"), AutomationRouteStepIndex, *Step.Target.ToCompactString());
            bAutomationRouteStepMoveLogged = true;
        }
        ++AutomationInputMoveSamples;
        Character->AddMovementInput(MoveDirection, 1.0f, true);
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->AddInputVector(MoveDirection, true);
            if (Movement->Velocity.Size2D() < 45.0f)
            {
                const FVector NextLocation = CharacterLocation + MoveDirection * Movement->MaxWalkSpeed * DeltaSeconds;
                Character->SetActorLocation(NextLocation, false, nullptr, ETeleportType::None);
            }
        }
        const float DesiredYaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
        Character->SetActorRotation(FRotator(0.0f, DesiredYaw, 0.0f));
        return;
    }

    if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    if (!bAutomationRouteStepActionFired)
    {
        UE_LOG(LogTemp, Display, TEXT("LW_ROUTE_STEP_REACHED index=%d target=%s"), AutomationRouteStepIndex, *Step.Target.ToCompactString());
        if (Step.bActivateTemple)
        {
            if (AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase))
            {
                if (Character->InvestTempleOffering())
                {
                    ++AutomationInputOfferingActions;
                    UE_LOG(LogTemp, Display, TEXT("LW_INPUT_ACTION offering step=%d"), AutomationRouteStepIndex);
                }
            }
            if (AThunderHammerTemple* Temple = FindThunderHammerTemple())
            {
                Temple->ActivateTemple(Character);
            }
        }
        if (Step.bStartFinalWave)
        {
            TryStartFinalWave();
            if (AutomationScenario.Equals(TEXT("first-temple-run-paced-proof"), ESearchCase::IgnoreCase))
            {
                ApplyAutomationSurvivalPressure(Character);
            }
        }
        if (Step.bBreakFinalWave)
        {
            BreakAutomationFinalWaveEnemies();
        }
        if (Step.GateImpCount > 0 || Step.GateBerserkerCount > 0)
        {
            SpawnAutomationCombatGate(Step, Character->GetActorLocation());
        }

        AutomationRouteHoldRemaining = Step.HoldSeconds;
        bAutomationRouteStepActionFired = true;
        if (Step.HoldSeconds > 0.0f)
        {
            UE_LOG(LogTemp, Display, TEXT("LW_INPUT_ACTION hold step=%d seconds=%.1f"), AutomationRouteStepIndex, Step.HoldSeconds);
        }
    }

    if (bAutomationCombatGateActive)
    {
        UpdateAutomationCombatGate(DeltaSeconds, Character);
        if (bAutomationCombatGateActive)
        {
            return;
        }
    }

    AutomationInputHoldSeconds += DeltaSeconds;
    AutomationRouteHoldRemaining -= DeltaSeconds;
    if (AutomationRouteHoldRemaining > 0.0f)
    {
        return;
    }

    ++AutomationRouteStepIndex;
    bAutomationRouteStepActionFired = false;
    bAutomationRouteStepMoveLogged = false;
    AutomationRouteHoldRemaining = 0.0f;

    if (!AutomationRouteSteps.IsValidIndex(AutomationRouteStepIndex))
    {
        bAutomationRouteActive = false;
    }
}

void ALightWarriorGameMode::SpawnAutomationCombatGate(const FAutomationRouteStep& Step, const FVector& GateCenter)
{
    ActiveAutomationCombatGateEnemies.Empty();
    AutomationCombatStrikeCooldownRemaining = 0.0f;
    bAutomationCombatGateActive = false;

    const int32 EnemyTotal = Step.GateImpCount + Step.GateBerserkerCount;
    if (EnemyTotal <= 0)
    {
        return;
    }

    ++AutomationCombatGatesStarted;

    int32 SpawnIndex = 1;
    for (int32 Index = 0; Index < Step.GateImpCount; ++Index)
    {
        const float Angle = (2.0f * PI * SpawnIndex) / FMath::Max(1, EnemyTotal);
        const FVector Offset(FMath::Cos(Angle) * 250.0f, FMath::Sin(Angle) * 250.0f, -50.0f);
        if (AShadowEnemy* Enemy = SpawnPressureEnemy(GateCenter + Offset, TEXT("LW_PacedGateImp"), SpawnIndex, EShadowEnemyArchetype::ShadowImp))
        {
            Enemy->BeginPressurePreview(1.0f);
            ActiveAutomationCombatGateEnemies.Add(Enemy);
        }
        ++SpawnIndex;
    }

    for (int32 Index = 0; Index < Step.GateBerserkerCount; ++Index)
    {
        const float Angle = (2.0f * PI * SpawnIndex) / FMath::Max(1, EnemyTotal);
        const FVector Offset(FMath::Cos(Angle) * 285.0f, FMath::Sin(Angle) * 285.0f, -50.0f);
        if (AShadowEnemy* Enemy = SpawnPressureEnemy(GateCenter + Offset, TEXT("LW_PacedGateBerserker"), SpawnIndex, EShadowEnemyArchetype::Berserker))
        {
            Enemy->BeginPressurePreview(1.25f);
            ActiveAutomationCombatGateEnemies.Add(Enemy);
        }
        ++SpawnIndex;
    }

    bAutomationCombatGateActive = GetActiveAutomationCombatGateEnemies() > 0;
    UE_LOG(
        LogTemp,
        Display,
        TEXT("LW_COMBAT_GATE_STARTED index=%d enemies=%d"),
        AutomationRouteStepIndex,
        GetActiveAutomationCombatGateEnemies());
}

int32 ALightWarriorGameMode::GetActiveAutomationCombatGateEnemies() const
{
    int32 ActiveCount = 0;
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : ActiveAutomationCombatGateEnemies)
    {
        if (Enemy.IsValid())
        {
            ++ActiveCount;
        }
    }
    return ActiveCount;
}

void ALightWarriorGameMode::UpdateAutomationCombatGate(float DeltaSeconds, ALightWarriorCharacter* Character)
{
    TArray<TWeakObjectPtr<AShadowEnemy>> InvalidEnemies;
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : ActiveAutomationCombatGateEnemies)
    {
        if (!Enemy.IsValid())
        {
            InvalidEnemies.Add(Enemy);
        }
    }
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : InvalidEnemies)
    {
        ActiveAutomationCombatGateEnemies.Remove(Enemy);
    }

    if (GetActiveAutomationCombatGateEnemies() <= 0)
    {
        bAutomationCombatGateActive = false;
        ++AutomationCombatGatesCompleted;
        UE_LOG(LogTemp, Display, TEXT("LW_COMBAT_GATE_COMPLETE index=%d completed=%d"), AutomationRouteStepIndex, AutomationCombatGatesCompleted);
        return;
    }

    if (!Character)
    {
        return;
    }

    AutomationCombatStrikeCooldownRemaining -= DeltaSeconds;
    if (AutomationCombatStrikeCooldownRemaining > 0.0f)
    {
        return;
    }

    AShadowEnemy* NearestEnemy = nullptr;
    float NearestDistanceSquared = TNumericLimits<float>::Max();
    for (const TWeakObjectPtr<AShadowEnemy>& EnemyPtr : ActiveAutomationCombatGateEnemies)
    {
        AShadowEnemy* Enemy = EnemyPtr.Get();
        if (!Enemy)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(Character->GetActorLocation(), Enemy->GetActorLocation());
        if (DistanceSquared < NearestDistanceSquared)
        {
            NearestDistanceSquared = DistanceSquared;
            NearestEnemy = Enemy;
        }
    }

    if (NearestEnemy)
    {
        const FVector ToEnemy = NearestEnemy->GetActorLocation() - Character->GetActorLocation();
        Character->SetActorRotation(FRotator(0.0f, FMath::RadiansToDegrees(FMath::Atan2(ToEnemy.Y, ToEnemy.X)), 0.0f));
    }

    Character->TriggerLightStrike();
    ++AutomationInputCombatActions;
    UE_LOG(LogTemp, Display, TEXT("LW_INPUT_ACTION light_strike gate=%d actions=%d"), AutomationRouteStepIndex, AutomationInputCombatActions);
    AutomationCombatStrikeCooldownRemaining = Character->HasThunderHammer() ? 0.55f : 0.78f;
}

void ALightWarriorGameMode::ApplyAutomationSurvivalPressure(ALightWarriorCharacter* Character)
{
    if (!Character || bAutomationSurvivalPressureApplied)
    {
        return;
    }

    bAutomationSurvivalPressureApplied = true;
    ++AutomationInputSurvivalActions;
    UGameplayStatics::ApplyDamage(Character, 34.0f, nullptr, this, UDamageType::StaticClass());
    UE_LOG(LogTemp, Display, TEXT("LW_INPUT_ACTION survive_pressure actions=%d"), AutomationInputSurvivalActions);
    UE_LOG(LogTemp, Display, TEXT("LW_SURVIVAL_PRESSURE_APPLIED amount=34 health=%.0f"), Character->GetHealth());
}

void ALightWarriorGameMode::MoveAutomationPlayerToLocation(const FVector& Location, float YawDegrees)
{
    if (ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        Character->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
        Character->SetActorRotation(FRotator(0.0f, YawDegrees, 0.0f));
    }
}

void ALightWarriorGameMode::BreakAutomationFinalWaveEnemies()
{
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AShadowEnemy> It(World); It; ++It)
    {
        UGameplayStatics::ApplyDamage(*It, 500.0f, Character ? Character->GetController() : nullptr, Character, UDamageType::StaticClass());
    }
}

ALightWell* ALightWarriorGameMode::FindAutomationLightWellNear(const FVector& TargetLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    ALightWell* BestWell = nullptr;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    for (TActorIterator<ALightWell> It(World); It; ++It)
    {
        if (It->IsPurified())
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(It->GetActorLocation(), TargetLocation);
        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestWell = *It;
        }
    }

    return BestWell;
}

void ALightWarriorGameMode::HandleLightWellPurificationStarted(ALightWell* LightWell)
{
    if (!LightWell || RunState != ELightWarriorRunState::Running || LightWellsWithPressure.Contains(LightWell))
    {
        return;
    }

    LightWellsWithPressure.Add(LightWell);
    SpawnLightWellPressure(LightWell, 3, 760.0f, TEXT("LW_LightWellPressure"));
}

void ALightWarriorGameMode::HandleLightWellPurified(ALightWell* LightWell)
{
    if (RunState != ELightWarriorRunState::Running)
    {
        return;
    }

    ++PurifiedLightWells;
    OnObjectiveProgressChanged.Broadcast(PurifiedLightWells, RequiredLightWells);

    if (PurifiedLightWells >= RequiredLightWells)
    {
        bTempleUnlocked = true;
        if (AThunderHammerTemple* Temple = FindThunderHammerTemple())
        {
            Temple->SetTempleUnlocked(true);
        }
        SpawnTempleUnlockRewards();
    }
}

void ALightWarriorGameMode::SpawnLightWellPressure(ALightWell* LightWell, int32 EnemyCount, float SpawnRadius, const TCHAR* LabelPrefix)
{
    if (!LightWell || !GetWorld())
    {
        return;
    }

    const FVector WellLocation = LightWell->GetActorLocation();
    const FVector TowardOrigin = (-FVector(WellLocation.X, WellLocation.Y, 0.0f)).GetSafeNormal();
    const FVector Tangent(-TowardOrigin.Y, TowardOrigin.X, 0.0f);

    for (int32 Index = 0; Index < EnemyCount; ++Index)
    {
        const float SideOffset = (Index - (EnemyCount - 1) * 0.5f) * 260.0f;
        const FVector SpawnLocation = WellLocation + TowardOrigin * SpawnRadius + Tangent * SideOffset + FVector(0.0f, 0.0f, 90.0f);
        const EShadowEnemyArchetype Archetype = Index == EnemyCount - 1 ? EShadowEnemyArchetype::Berserker : EShadowEnemyArchetype::ShadowImp;
        SpawnPressureEnemy(SpawnLocation, LabelPrefix, Index + 1, Archetype);
    }
}

AShadowEnemy* ALightWarriorGameMode::SpawnPressureEnemy(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index, EShadowEnemyArchetype Archetype)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AShadowEnemy* Enemy = GetWorld()->SpawnActor<AShadowEnemy>(AShadowEnemy::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Enemy)
    {
        Enemy->ConfigureArchetype(Archetype);
        Enemy->SetActorLabel(FString::Printf(TEXT("%s_%02d"), LabelPrefix, Index));
    }

    return Enemy;
}

void ALightWarriorGameMode::SpawnLightShard(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index)
{
    if (!GetWorld())
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ALightShardPickup* Shard = GetWorld()->SpawnActor<ALightShardPickup>(ALightShardPickup::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Shard)
    {
        Shard->SetActorLabel(FString::Printf(TEXT("%s_%02d"), LabelPrefix, Index));
    }
}

void ALightWarriorGameMode::SpawnRouteSegment(const FVector& Start, const FVector& End, const FLinearColor& Color, const TCHAR* Label)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UStaticMesh* PathMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!PathMesh)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    const FVector Delta = End - Start;
    const FVector Midpoint = Start + Delta * 0.5f + FVector(0.0f, 0.0f, -34.0f);
    const float Length = Delta.Size2D();
    const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));

    AStaticMeshActor* Segment = World->SpawnActor<AStaticMeshActor>(Midpoint, FRotator(0.0f, Yaw, 0.0f), SpawnParams);
    if (!Segment)
    {
        return;
    }

    Segment->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    Segment->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
    Segment->SetActorScale3D(FVector(FMath::Max(1.0f, Length / 100.0f), 0.46f, 0.035f));
    ApplyMeshColor(Segment->GetStaticMeshComponent(), Color * 0.62f, false);
    Segment->SetActorLabel(Label);
}

void ALightWarriorGameMode::SpawnBeaconLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const TCHAR* Label)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    APointLight* Beacon = World->SpawnActor<APointLight>(Location, FRotator::ZeroRotator, SpawnParams);
    if (Beacon && Beacon->PointLightComponent)
    {
        Beacon->PointLightComponent->SetLightColor(Color);
        Beacon->PointLightComponent->SetIntensity(Intensity);
        Beacon->PointLightComponent->SetAttenuationRadius(Radius);
        Beacon->SetActorLabel(Label);
    }
}

void ALightWarriorGameMode::SpawnTempleUnlockRewards()
{
    if (!GetWorld())
    {
        return;
    }

    AThunderHammerTemple* Temple = FindThunderHammerTemple();

    if (!Temple)
    {
        return;
    }

    const FVector TempleLocation = Temple->GetActorLocation();
    const FVector RouteStart(0.0f, 420.0f, 0.0f);
    const FVector RouteMid(0.0f, 1320.0f, 0.0f);
    const FVector RouteEnd = TempleLocation + FVector(0.0f, -320.0f, 0.0f);

    SpawnRouteSegment(RouteStart, RouteMid, FLinearColor(0.30f, 0.95f, 1.0f), TEXT("LW_AwakenedTempleRoute_01"));
    SpawnRouteSegment(RouteMid, RouteEnd, FLinearColor(1.0f, 0.82f, 0.24f), TEXT("LW_AwakenedTempleRoute_02"));
    SpawnBeaconLight(RouteStart + FVector(0.0f, 0.0f, 190.0f), FLinearColor(0.30f, 0.82f, 0.92f), 1800.0f, 680.0f, TEXT("LW_TempleRouteBeacon_01"));
    SpawnBeaconLight(RouteMid + FVector(0.0f, 0.0f, 205.0f), FLinearColor(0.92f, 0.70f, 0.22f), 2400.0f, 760.0f, TEXT("LW_TempleRouteBeacon_02"));
    SpawnBeaconLight(TempleLocation + FVector(0.0f, -260.0f, 265.0f), FLinearColor(0.95f, 0.62f, 0.16f), 4200.0f, 980.0f, TEXT("LW_TempleAwakeBeacon"));

    SpawnLightShard(RouteStart + FVector(-130.0f, 260.0f, 90.0f), TEXT("LW_TempleUnlockShard"), 1);
    SpawnLightShard(RouteMid + FVector(150.0f, -140.0f, 90.0f), TEXT("LW_TempleUnlockShard"), 2);
    SpawnLightShard(TempleLocation + FVector(-260.0f, -360.0f, 90.0f), TEXT("LW_TempleUnlockShard"), 3);
    SpawnLightShard(TempleLocation + FVector(260.0f, -360.0f, 90.0f), TEXT("LW_TempleUnlockShard"), 4);
    SpawnLightShard(TempleLocation + FVector(0.0f, -680.0f, 90.0f), TEXT("LW_TempleUnlockShard"), 5);
}

AThunderHammerTemple* ALightWarriorGameMode::FindThunderHammerTemple() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (TActorIterator<AThunderHammerTemple> It(World); It; ++It)
    {
        return *It;
    }

    return nullptr;
}

FVector ALightWarriorGameMode::GetFinalWaveCenter() const
{
    if (const AThunderHammerTemple* Temple = FindThunderHammerTemple())
    {
        return Temple->GetActorLocation() + FVector(0.0f, -660.0f, 0.0f);
    }

    return FVector(0.0f, 1540.0f, 0.0f);
}

void ALightWarriorGameMode::TryStartFinalWave()
{
    if (RunState != ELightWarriorRunState::Running || bFinalWaveStarted || PurifiedLightWells < RequiredLightWells)
    {
        return;
    }

    const ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Character && Character->HasThunderHammer())
    {
        StartFinalWave();
    }
}

void ALightWarriorGameMode::StartFinalWave()
{
    if (!GetWorld() || bFinalWaveStarted)
    {
        return;
    }

    bFinalWaveStarted = true;
    FinalWaveRemainingSeconds = FinalWaveDurationSeconds;
    ActiveFinalWaveEnemies.Empty();

    const FVector Center = GetFinalWaveCenter();
    int32 SpawnIndex = 1;

    for (int32 Index = 0; Index < FinalWaveImpCount; ++Index)
    {
        const float Angle = (-145.0f + 72.0f * Index) * PI / 180.0f;
        const FVector Offset(FMath::Cos(Angle) * 640.0f, FMath::Sin(Angle) * 420.0f, 90.0f);
        if (AShadowEnemy* Enemy = SpawnPressureEnemy(Center + Offset, TEXT("LW_FinalWaveImp"), SpawnIndex++, EShadowEnemyArchetype::ShadowImp))
        {
            Enemy->BeginPressurePreview(FinalWaveEntryPreviewSeconds);
            if (Index == 0)
            {
                Enemy->PulseHitFeedback(0.65f);
            }
            ActiveFinalWaveEnemies.Add(Enemy);
            Enemy->OnDestroyed.AddDynamic(this, &ALightWarriorGameMode::HandleFinalWaveEnemyDestroyed);
        }
    }

    for (int32 Index = 0; Index < FinalWaveBerserkerCount; ++Index)
    {
        const float Side = Index % 2 == 0 ? -1.0f : 1.0f;
        const FVector Offset(Side * 760.0f, -120.0f + Index * 320.0f, 90.0f);
        if (AShadowEnemy* Enemy = SpawnPressureEnemy(Center + Offset, TEXT("LW_FinalWaveBerserker"), Index + 1, EShadowEnemyArchetype::Berserker))
        {
            Enemy->BeginPressurePreview(FinalWaveEntryPreviewSeconds);
            if (Index == 0)
            {
                Enemy->PulseHitFeedback(0.65f);
            }
            ActiveFinalWaveEnemies.Add(Enemy);
            Enemy->OnDestroyed.AddDynamic(this, &ALightWarriorGameMode::HandleFinalWaveEnemyDestroyed);
        }
    }
}

void ALightWarriorGameMode::UpdateFinalWave(float DeltaSeconds)
{
    if (!bFinalWaveStarted || bFinalWaveCleared || RunState != ELightWarriorRunState::Running)
    {
        return;
    }

    FinalWaveRemainingSeconds = FMath::Max(0.0f, FinalWaveRemainingSeconds - DeltaSeconds);

    TArray<TWeakObjectPtr<AShadowEnemy>> InvalidEnemies;
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : ActiveFinalWaveEnemies)
    {
        if (!Enemy.IsValid())
        {
            InvalidEnemies.Add(Enemy);
        }
    }
    for (const TWeakObjectPtr<AShadowEnemy>& Enemy : InvalidEnemies)
    {
        ActiveFinalWaveEnemies.Remove(Enemy);
    }

    if (GetActiveFinalWaveEnemies() <= 0 || FinalWaveRemainingSeconds <= 0.0f)
    {
        bFinalWaveCleared = true;
        CompleteRun(true);
    }
}

void ALightWarriorGameMode::HandleFinalWaveEnemyDestroyed(AActor* DestroyedActor)
{
    if (AShadowEnemy* Enemy = Cast<AShadowEnemy>(DestroyedActor))
    {
        ActiveFinalWaveEnemies.Remove(Enemy);
    }
}

void ALightWarriorGameMode::SetRunState(ELightWarriorRunState NewState)
{
    if (RunState == NewState)
    {
        return;
    }

    RunState = NewState;
    OnRunStateChanged.Broadcast(RunState);
}
