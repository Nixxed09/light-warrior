#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LightWarriorAudio.generated.h"

UENUM(BlueprintType)
enum class ELightWarriorSfx : uint8
{
    Dash,
    LightStrike,
    ShadowDissolve,
    CircleExpand,
    TempleActivate,
    HammerSlam
};

UCLASS()
class LIGHTWARRIOR_API ULightWarriorAudio : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Light Warrior|Audio", meta = (WorldContext = "WorldContextObject"))
    static void PlaySfx(const UObject* WorldContextObject, ELightWarriorSfx Sfx, FVector Location, float Volume = 1.0f);
};
