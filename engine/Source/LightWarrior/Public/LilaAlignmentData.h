#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LilaAlignmentData.generated.h"

UENUM(BlueprintType)
enum class ELightWarriorLilaLaw : uint8
{
    Resonance,
    Poles,
    Consent,
    Mirrors,
    Arising,
    Remembering
};

UENUM(BlueprintType)
enum class ELightWarriorOctaveBand : uint8
{
    Asleep,
    Brave,
    Clear,
    Radiant
};

USTRUCT(BlueprintType)
struct FLightWarriorLilaLawBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    ELightWarriorLilaLaw Law = ELightWarriorLilaLaw::Resonance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FText CombatExpression;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FText ProgressionExpression;
};

USTRUCT(BlueprintType)
struct FLightWarriorOctaveRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    ELightWarriorOctaveBand Band = ELightWarriorOctaveBand::Asleep;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FText PlayerRead;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FText WorldState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    float MinimumCircleRadius = 0.0f;
};

UCLASS(BlueprintType)
class LIGHTWARRIOR_API ULilaAlignmentData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FName AlignmentId = TEXT("light-warrior");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    FText CoreHook;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    TArray<FLightWarriorLilaLawBinding> ActiveLaws;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    TArray<FLightWarriorOctaveRule> OctaveRules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lila")
    bool bRuntimeAiEnabled = false;
};
