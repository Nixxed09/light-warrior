#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShadowEnemy.h"
#include "LightWarriorGameMode.generated.h"

UENUM(BlueprintType)
enum class ELightWarriorRunState : uint8
{
    Waiting,
    Running,
    Victory,
    Failure
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLightWarriorRunStateChanged, ELightWarriorRunState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLightWarriorRunTimeChanged, float, RemainingSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLightWarriorObjectiveProgressChanged, int32, PurifiedLightWells, int32, RequiredLightWells);

UCLASS()
class LIGHTWARRIOR_API ALightWarriorGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALightWarriorGameMode();

    virtual void BeginPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Run")
    void StartRun();

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Run")
    void CompleteRun(bool bVictory);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Run")
    ELightWarriorRunState GetRunState() const { return RunState; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Run")
    float GetRemainingRunSeconds() const { return RemainingRunSeconds; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    int32 GetPurifiedLightWells() const { return PurifiedLightWells; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    int32 GetRequiredLightWells() const { return RequiredLightWells; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    bool IsTempleUnlocked() const { return bTempleUnlocked; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    bool IsFinalWaveStarted() const { return bFinalWaveStarted; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    float GetFinalWaveRemainingSeconds() const { return FinalWaveRemainingSeconds; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    float GetFinalWaveDurationSeconds() const { return FinalWaveDurationSeconds; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Objectives")
    int32 GetActiveFinalWaveEnemies() const;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunStateChanged OnRunStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunTimeChanged OnRunTimeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Objectives")
    FLightWarriorObjectiveProgressChanged OnObjectiveProgressChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    float RunDurationSeconds = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    bool bAutoStartRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    bool bRestartAfterFailure = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    float FailureRestartDelaySeconds = 1.25f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    ELightWarriorRunState RunState = ELightWarriorRunState::Waiting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    float RemainingRunSeconds = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    int32 RequiredLightWells = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Objectives")
    int32 PurifiedLightWells = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    float FinalWaveDurationSeconds = 68.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    int32 FinalWaveImpCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    int32 FinalWaveBerserkerCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    float FinalWaveEntryPreviewSeconds = 7.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Arena")
    bool bAutoSpawnArenaIfEmpty = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Arena")
    int32 AutoSpawnEnemyCount = 5;

private:
    UFUNCTION()
    void HandleLightWellPurified(class ALightWell* LightWell);

    UFUNCTION()
    void HandleLightWellPurificationStarted(class ALightWell* LightWell);

    UFUNCTION()
    void HandleFinalWaveEnemyDestroyed(AActor* DestroyedActor);

    void BootstrapPlayableArenaIfNeeded();
    void BindLightWellObjectives();
    void SpawnLightWellPressure(class ALightWell* LightWell, int32 EnemyCount, float SpawnRadius, const TCHAR* LabelPrefix);
    AShadowEnemy* SpawnPressureEnemy(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index, EShadowEnemyArchetype Archetype = EShadowEnemyArchetype::ShadowImp);
    void SpawnLightShard(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index);
    void SpawnRouteSegment(const FVector& Start, const FVector& End, const FLinearColor& Color, const TCHAR* Label);
    void SpawnBeaconLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const TCHAR* Label);
    void SpawnTempleUnlockRewards();
    class AThunderHammerTemple* FindThunderHammerTemple() const;
    FVector GetFinalWaveCenter() const;
    void TryStartFinalWave();
    void StartFinalWave();
    void UpdateFinalWave(float DeltaSeconds);
    void PlaceAutomationPlayerAtFirstLightWell();
    void PlaceAutomationPlayerInCombat();
    void StageAutomationThunderHammer(bool bSpawnShowcaseEnemies);
    void PlaceAutomationPlayerAtThunderHammer();
    void PlaceAutomationPlayerAtFirstTempleRun();
    void StartAutomationFirstTempleRunProof();
    void StartAutomationFirstTempleRunRouteProof();
    void StartAutomationFirstTempleRunPacedProof();
    void UpdateAutomationRoute(float DeltaSeconds);
    void MoveAutomationPlayerToLocation(const FVector& Location, float YawDegrees);
    void BreakAutomationFinalWaveEnemies();
    class ALightWell* FindAutomationLightWellNear(const FVector& TargetLocation) const;
    void ConfigureAutomationLoop();
    void CaptureAutomationScreenshot();
    void QuitAfterAutomationCapture();
    bool IsPlayerDefeated() const;
    void RestartLevelAfterFailure();
    void SetRunState(ELightWarriorRunState NewState);

    FString AutomationScreenshotPath;
    FString AutomationScenario;

    struct FAutomationRouteStep
    {
        FVector Target = FVector::ZeroVector;
        float AcceptanceRadius = 160.0f;
        float HoldSeconds = 0.0f;
        bool bActivateTemple = false;
        bool bStartFinalWave = false;
        bool bBreakFinalWave = false;
        int32 GateImpCount = 0;
        int32 GateBerserkerCount = 0;
    };

    void SpawnAutomationCombatGate(const FAutomationRouteStep& Step, const FVector& GateCenter);
    int32 GetActiveAutomationCombatGateEnemies() const;
    void UpdateAutomationCombatGate(float DeltaSeconds, class ALightWarriorCharacter* Character);
    void ApplyAutomationSurvivalPressure(class ALightWarriorCharacter* Character);

    TArray<FAutomationRouteStep> AutomationRouteSteps;
    int32 AutomationRouteStepIndex = INDEX_NONE;
    float AutomationRouteHoldRemaining = 0.0f;
    float AutomationRouteStartWorldSeconds = 0.0f;
    float AutomationCombatStrikeCooldownRemaining = 0.0f;
    float AutomationInputHoldSeconds = 0.0f;
    int32 AutomationInputMoveSamples = 0;
    int32 AutomationInputCombatActions = 0;
    int32 AutomationInputOfferingActions = 0;
    int32 AutomationInputSurvivalActions = 0;
    int32 AutomationCombatGatesStarted = 0;
    int32 AutomationCombatGatesCompleted = 0;
    bool bAutomationRouteActive = false;
    bool bAutomationRouteStepActionFired = false;
    bool bAutomationRouteStepMoveLogged = false;
    bool bAutomationCombatGateActive = false;
    bool bAutomationSurvivalPressureApplied = false;

    TSet<TWeakObjectPtr<class ALightWell>> LightWellsWithPressure;
    TSet<TWeakObjectPtr<AShadowEnemy>> ActiveFinalWaveEnemies;
    TSet<TWeakObjectPtr<AShadowEnemy>> ActiveAutomationCombatGateEnemies;
    bool bTempleUnlocked = false;
    bool bFinalWaveStarted = false;
    bool bFinalWaveCleared = false;
    float FinalWaveRemainingSeconds = 0.0f;
};
