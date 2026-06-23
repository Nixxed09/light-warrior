#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunStateChanged OnRunStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunTimeChanged OnRunTimeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Objectives")
    FLightWarriorObjectiveProgressChanged OnObjectiveProgressChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    float RunDurationSeconds = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    bool bAutoStartRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    bool bRestartAfterFailure = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    float FailureRestartDelaySeconds = 1.25f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    ELightWarriorRunState RunState = ELightWarriorRunState::Waiting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    float RemainingRunSeconds = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Objectives")
    int32 RequiredLightWells = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Objectives")
    int32 PurifiedLightWells = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Arena")
    bool bAutoSpawnArenaIfEmpty = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Arena")
    int32 AutoSpawnEnemyCount = 5;

private:
    UFUNCTION()
    void HandleLightWellPurified(class ALightWell* LightWell);

    UFUNCTION()
    void HandleLightWellPurificationStarted(class ALightWell* LightWell);

    void BootstrapPlayableArenaIfNeeded();
    void BindLightWellObjectives();
    void SpawnLightWellPressure(class ALightWell* LightWell, int32 EnemyCount, float SpawnRadius, const TCHAR* LabelPrefix);
    void SpawnPressureEnemy(const FVector& SpawnLocation, const TCHAR* LabelPrefix, int32 Index);
    void PlaceAutomationPlayerAtFirstLightWell();
    void PlaceAutomationPlayerInCombat();
    void ConfigureAutomationLoop();
    void CaptureAutomationScreenshot();
    void QuitAfterAutomationCapture();
    bool IsPlayerDefeated() const;
    void RestartLevelAfterFailure();
    void SetRunState(ELightWarriorRunState NewState);

    FString AutomationScreenshotPath;
    FString AutomationScenario;
    TSet<TWeakObjectPtr<class ALightWell>> LightWellsWithPressure;
};
