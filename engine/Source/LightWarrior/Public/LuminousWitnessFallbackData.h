#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LuminousWitnessFallbackData.generated.h"

UENUM(BlueprintType)
enum class ELuminousWitnessVerb : uint8
{
    Comment,
    Hint,
    Warn,
    Celebrate,
    MarkOptionalLocation
};

UENUM(BlueprintType)
enum class ELuminousWitnessBeat : uint8
{
    StayedInSafety,
    LeftCircle,
    TookDarknessDamage,
    ActivatedThunderHammer,
    ExpandedCircle,
    Victory,
    Failure
};

USTRUCT(BlueprintType)
struct FLuminousWitnessFallbackLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    ELuminousWitnessBeat Beat = ELuminousWitnessBeat::ExpandedCircle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    ELuminousWitnessVerb Verb = ELuminousWitnessVerb::Comment;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    FText Line;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    FName OptionalTarget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    int32 Priority = 0;
};

UCLASS(BlueprintType)
class LIGHTWARRIOR_API ULuminousWitnessFallbackData : public UDataAsset
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Luminous Witness")
    bool GetLineForBeat(ELuminousWitnessBeat Beat, FLuminousWitnessFallbackLine& OutLine) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luminous Witness")
    TArray<FLuminousWitnessFallbackLine> Lines;
};
