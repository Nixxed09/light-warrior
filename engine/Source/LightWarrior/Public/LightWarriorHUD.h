#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LightWarriorHUD.generated.h"

UCLASS()
class LIGHTWARRIOR_API ALightWarriorHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    FString FormatRunState(class ALightWarriorGameMode const* GameMode) const;
    FString FormatTime(float Seconds) const;
    FString BuildObjectiveLine(class ALightWarriorGameMode const* GameMode) const;
    float GetBestLightWellProgress() const;
    void DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& Color) const;
    void DrawBar(const FString& Label, float X, float Y, float Width, float Height, float Percent, const FLinearColor& FillColor) const;
    void DrawValuePill(const FString& Label, const FString& Value, float X, float Y, float Width, const FLinearColor& AccentColor) const;
    void DrawCenterReticle() const;
    void DrawShadowText(const FString& Text, float X, float Y, const FLinearColor& Color, float Scale = 1.0f) const;
};
