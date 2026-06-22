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

UCLASS()
class LIGHTWARRIOR_API ALightWarriorGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALightWarriorGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Run")
    void StartRun();

    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Run")
    void CompleteRun(bool bVictory);

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Run")
    ELightWarriorRunState GetRunState() const { return RunState; }

    UFUNCTION(BlueprintPure, Category = "Light Warrior|Run")
    float GetRemainingRunSeconds() const { return RemainingRunSeconds; }

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunStateChanged OnRunStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Light Warrior|Run")
    FLightWarriorRunTimeChanged OnRunTimeChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    float RunDurationSeconds = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Warrior|Run")
    bool bAutoStartRun = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    ELightWarriorRunState RunState = ELightWarriorRunState::Waiting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light Warrior|Run")
    float RemainingRunSeconds = 90.0f;

private:
    bool IsPlayerDefeated() const;
    void SetRunState(ELightWarriorRunState NewState);
};
