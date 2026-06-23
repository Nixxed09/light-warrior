#include "LightWarriorGameMode.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "LightWell.h"
#include "LightWarriorCharacter.h"
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

    const FTransform SpawnTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(-420.0f, 0.0f, 140.0f));
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
        CompleteRun(false);
        return;
    }

    RemainingRunSeconds = FMath::Max(0.0f, RemainingRunSeconds - DeltaSeconds);
    OnRunTimeChanged.Broadcast(RemainingRunSeconds);

    if (RemainingRunSeconds <= 0.0f)
    {
        CompleteRun(true);
    }
}

void ALightWarriorGameMode::StartRun()
{
    PurifiedLightWells = 0;
    RemainingRunSeconds = RunDurationSeconds;
    OnObjectiveProgressChanged.Broadcast(PurifiedLightWells, RequiredLightWells);
    OnRunTimeChanged.Broadcast(RemainingRunSeconds);
    SetRunState(ELightWarriorRunState::Running);
}

void ALightWarriorGameMode::CompleteRun(bool bVictory)
{
    SetRunState(bVictory ? ELightWarriorRunState::Victory : ELightWarriorRunState::Failure);

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
                Path->SetActorScale3D(FVector(Length / 100.0f, 2.4f, 0.045f));
                ApplyMeshColor(Path->GetStaticMeshComponent(), FLinearColor(0.22f, 0.92f, 1.0f), true);
                Path->SetActorLabel(TEXT("LW_LightPath"));
            }

            APointLight* PathLight = World->SpawnActor<APointLight>(Midpoint + FVector(0.0f, 0.0f, 190.0f), FRotator::ZeroRotator, SpawnParams);
            if (PathLight && PathLight->PointLightComponent)
            {
                PathLight->PointLightComponent->SetLightColor(FLinearColor(0.38f, 0.94f, 1.0f));
                PathLight->PointLightComponent->SetIntensity(2600.0f);
                PathLight->PointLightComponent->SetAttenuationRadius(900.0f);
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
    }

    SpawnPressureEnemy(FVector(910.0f, -260.0f, 90.0f), TEXT("LW_FirstWellSentinel"), 1, EShadowEnemyArchetype::ShadowImp);
    SpawnPressureEnemy(FVector(1030.0f, 300.0f, 90.0f), TEXT("LW_FirstWellSentinel"), 2, EShadowEnemyArchetype::Berserker);

    AThunderHammerTemple* Temple = World->SpawnActor<AThunderHammerTemple>(AThunderHammerTemple::StaticClass(), FVector(0.0f, 2200.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
    if (Temple)
    {
        Temple->SetActorLabel(TEXT("LW_ThunderHammerTemple"));
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
    UWorld* World = GetWorld();
    ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!World || !Character)
    {
        return;
    }

    AThunderHammerTemple* Temple = nullptr;
    for (TActorIterator<AThunderHammerTemple> It(World); It; ++It)
    {
        Temple = *It;
        break;
    }

    if (!Temple)
    {
        return;
    }

    const FVector TempleLocation = Temple->GetActorLocation();
    Character->SetActorLocation(TempleLocation + FVector(-120.0f, -80.0f, 140.0f), false, nullptr, ETeleportType::TeleportPhysics);
    Character->SetActorRotation(FRotator(0.0f, 45.0f, 0.0f));
    Temple->ActivateTemple(Character);

    SpawnPressureEnemy(TempleLocation + FVector(520.0f, 90.0f, 90.0f), TEXT("LW_HammerImp"), 1, EShadowEnemyArchetype::ShadowImp);
    SpawnPressureEnemy(TempleLocation + FVector(-520.0f, -160.0f, 90.0f), TEXT("LW_HammerBerserker"), 1, EShadowEnemyArchetype::Berserker);
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
        CompleteRun(true);
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

void ALightWarriorGameMode::SpawnPressureEnemy(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index, EShadowEnemyArchetype Archetype)
{
    if (!GetWorld())
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AShadowEnemy* Enemy = GetWorld()->SpawnActor<AShadowEnemy>(AShadowEnemy::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Enemy)
    {
        Enemy->ConfigureArchetype(Archetype);
        Enemy->SetActorLabel(FString::Printf(TEXT("%s_%02d"), LabelPrefix, Index));
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
