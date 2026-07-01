#include "LightWarriorAudio.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundWaveProcedural.h"
#include "TimerManager.h"

namespace
{
struct FLightWarriorTone
{
    float Duration = 0.2f;
    float FrequencyA = 440.0f;
    float FrequencyB = 660.0f;
    float Bend = 0.0f;
    float Noise = 0.0f;
    float Attack = 0.01f;
    float Release = 0.14f;
    float Volume = 0.55f;
};

FLightWarriorTone GetTone(ELightWarriorSfx Sfx)
{
    switch (Sfx)
    {
    case ELightWarriorSfx::Dash:
        return {0.18f, 520.0f, 1040.0f, 360.0f, 0.08f, 0.006f, 0.16f, 0.34f};
    case ELightWarriorSfx::LightStrike:
        return {0.24f, 620.0f, 1480.0f, -220.0f, 0.05f, 0.004f, 0.20f, 0.48f};
    case ELightWarriorSfx::ShadowDissolve:
        return {0.38f, 180.0f, 720.0f, 260.0f, 0.22f, 0.012f, 0.32f, 0.46f};
    case ELightWarriorSfx::CircleExpand:
        return {0.72f, 220.0f, 880.0f, 320.0f, 0.03f, 0.035f, 0.58f, 0.44f};
    case ELightWarriorSfx::TempleActivate:
        return {0.96f, 140.0f, 560.0f, 520.0f, 0.06f, 0.025f, 0.78f, 0.62f};
    case ELightWarriorSfx::HammerSlam:
        return {0.54f, 72.0f, 260.0f, -90.0f, 0.18f, 0.003f, 0.42f, 0.78f};
    default:
        return {};
    }
}

const TCHAR* GetProductionSoundPath(ELightWarriorSfx Sfx)
{
    switch (Sfx)
    {
    case ELightWarriorSfx::Dash:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/dash_shimmer.dash_shimmer");
    case ELightWarriorSfx::LightStrike:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/light_strike.light_strike");
    case ELightWarriorSfx::ShadowDissolve:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/shadow_dissolve.shadow_dissolve");
    case ELightWarriorSfx::CircleExpand:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/circle_expansion.circle_expansion");
    case ELightWarriorSfx::TempleActivate:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/temple_activation.temple_activation");
    case ELightWarriorSfx::HammerSlam:
        return TEXT("/Game/LightWarrior/Audio/CoreSfx/hammer_slam.hammer_slam");
    default:
        return nullptr;
    }
}

USoundBase* LoadProductionSound(ELightWarriorSfx Sfx)
{
    const TCHAR* SoundPath = GetProductionSoundPath(Sfx);
    if (!SoundPath)
    {
        return nullptr;
    }

    return Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, SoundPath));
}

float Envelope(float Time, const FLightWarriorTone& Tone)
{
    const float AttackAlpha = FMath::Clamp(Time / FMath::Max(0.001f, Tone.Attack), 0.0f, 1.0f);
    const float ReleaseStart = FMath::Max(Tone.Attack, Tone.Duration - Tone.Release);
    const float ReleaseAlpha = Time > ReleaseStart
        ? 1.0f - FMath::Clamp((Time - ReleaseStart) / FMath::Max(0.001f, Tone.Release), 0.0f, 1.0f)
        : 1.0f;
    return FMath::Sin(AttackAlpha * HALF_PI) * FMath::Square(ReleaseAlpha);
}

USoundWaveProcedural* BuildProceduralWave(UObject* Outer, const FLightWarriorTone& Tone, float Volume)
{
    constexpr int32 SampleRate = 44100;
    constexpr int32 ChannelCount = 1;
    const int32 SampleCount = FMath::Max(1, FMath::RoundToInt(Tone.Duration * SampleRate));

    TArray<int16> Pcm;
    Pcm.SetNumUninitialized(SampleCount * ChannelCount);

    FRandomStream NoiseStream(1337);
    float PhaseA = 0.0f;
    float PhaseB = 0.0f;
    for (int32 Index = 0; Index < SampleCount; ++Index)
    {
        const float Time = static_cast<float>(Index) / static_cast<float>(SampleRate);
        const float Alpha = Time / FMath::Max(0.001f, Tone.Duration);
        const float FrequencyA = FMath::Max(20.0f, Tone.FrequencyA + Tone.Bend * Alpha);
        const float FrequencyB = FMath::Max(20.0f, Tone.FrequencyB + Tone.Bend * 0.5f * Alpha);
        PhaseA += UE_TWO_PI * FrequencyA / SampleRate;
        PhaseB += UE_TWO_PI * FrequencyB / SampleRate;

        const float Harmonic = FMath::Sin(PhaseA) * 0.72f + FMath::Sin(PhaseB) * 0.28f;
        const float Noise = NoiseStream.FRandRange(-1.0f, 1.0f) * Tone.Noise;
        const float Value = FMath::Clamp((Harmonic + Noise) * Envelope(Time, Tone) * Tone.Volume * Volume, -0.95f, 0.95f);
        Pcm[Index] = static_cast<int16>(Value * 32767.0f);
    }

    USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(Outer);
    Wave->SetSampleRate(SampleRate);
    Wave->NumChannels = ChannelCount;
    Wave->Duration = Tone.Duration;
    Wave->SoundGroup = SOUNDGROUP_Effects;
    Wave->bLooping = false;
    Wave->QueueAudio(reinterpret_cast<uint8*>(Pcm.GetData()), Pcm.Num() * sizeof(int16));
    return Wave;
}
}

void ULightWarriorAudio::PlaySfx(const UObject* WorldContextObject, ELightWarriorSfx Sfx, FVector Location, float Volume)
{
    if (!WorldContextObject)
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    if (USoundBase* ProductionSound = LoadProductionSound(Sfx))
    {
        UGameplayStatics::PlaySoundAtLocation(WorldContextObject, ProductionSound, Location, Volume, 1.0f, 0.0f);
        return;
    }

    const FLightWarriorTone Tone = GetTone(Sfx);
    USoundWaveProcedural* Wave = BuildProceduralWave(GetTransientPackage(), Tone, Volume);
    if (!Wave)
    {
        return;
    }

    Wave->AddToRoot();
    UGameplayStatics::PlaySoundAtLocation(WorldContextObject, Wave, Location, 1.0f, 1.0f, 0.0f);

    FTimerHandle CleanupHandle;
    TWeakObjectPtr<USoundWaveProcedural> WeakWave(Wave);
    World->GetTimerManager().SetTimer(
        CleanupHandle,
        [WeakWave]()
        {
            if (WeakWave.IsValid())
            {
                WeakWave->RemoveFromRoot();
            }
        },
        Tone.Duration + 1.0f,
        false);
}
