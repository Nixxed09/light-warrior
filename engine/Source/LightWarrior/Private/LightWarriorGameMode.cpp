#include "LightWarriorGameMode.h"

#include "LightWarriorCharacter.h"
#include "Kismet/GameplayStatics.h"

ALightWarriorGameMode::ALightWarriorGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = ALightWarriorCharacter::StaticClass();
    RemainingRunSeconds = RunDurationSeconds;
}

void ALightWarriorGameMode::BeginPlay()
{
    Super::BeginPlay();

    RemainingRunSeconds = RunDurationSeconds;
    if (bAutoStartRun)
    {
        StartRun();
    }
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
    RemainingRunSeconds = RunDurationSeconds;
    OnRunTimeChanged.Broadcast(RemainingRunSeconds);
    SetRunState(ELightWarriorRunState::Running);
}

void ALightWarriorGameMode::CompleteRun(bool bVictory)
{
    SetRunState(bVictory ? ELightWarriorRunState::Victory : ELightWarriorRunState::Failure);
}

bool ALightWarriorGameMode::IsPlayerDefeated() const
{
    const ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    return Character && Character->IsDefeated();
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
