#include "LightWarriorHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "LightWarriorCharacter.h"
#include "LightWarriorGameMode.h"
#include "LightWarriorProgressionComponent.h"
#include "LightWell.h"

void ALightWarriorHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas)
    {
        return;
    }

    const ALightWarriorCharacter* Character = Cast<ALightWarriorCharacter>(GetOwningPawn());
    const ALightWarriorGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALightWarriorGameMode>() : nullptr;

    const float Padding = 28.0f;
    const float ViewW = Canvas->ClipX;
    const float ViewH = Canvas->ClipY;
    const float PanelWidth = FMath::Clamp(ViewW * 0.30f, 360.0f, 460.0f);
    const float PanelHeight = 214.0f;

    DrawPanel(Padding - 12.0f, Padding - 14.0f, PanelWidth, PanelHeight, FLinearColor(0.014f, 0.019f, 0.034f, 0.84f));

    DrawShadowText(TEXT("LIGHT WARRIOR"), Padding, Padding, FLinearColor(0.88f, 0.96f, 1.0f), 1.18f);
    DrawShadowText(FormatRunState(GameMode), Padding, Padding + 25.0f, FLinearColor(1.0f, 0.78f, 0.28f), 0.78f);

    const float HealthPercent = Character ? FMath::Clamp(Character->GetHealthPercent(), 0.0f, 1.0f) : 0.0f;
    DrawBar(TEXT("Health"), Padding, Padding + 52.0f, PanelWidth - 138.0f, 14.0f, HealthPercent, FLinearColor(0.16f, 0.92f, 0.70f));

    const int32 Purified = GameMode ? GameMode->GetPurifiedLightWells() : 0;
    const int32 Required = GameMode ? GameMode->GetRequiredLightWells() : 3;
    const float Remaining = GameMode ? GameMode->GetRemainingRunSeconds() : 0.0f;
    DrawValuePill(TEXT("Wells"), FString::Printf(TEXT("%d/%d"), Purified, Required), Padding, Padding + 82.0f, 118.0f, FLinearColor(0.48f, 0.84f, 1.0f));
    DrawValuePill(TEXT("Time"), FormatTime(Remaining), Padding + 130.0f, Padding + 82.0f, 124.0f, FLinearColor(1.0f, 0.78f, 0.28f));

    FString PowerText = TEXT("Power: Light Strike");
    if (Character && Character->HasThunderHammer())
    {
        PowerText = FString::Printf(TEXT("Power: THUNDER HAMMER %.0fs"), Character->GetThunderHammerRemaining());
    }
    DrawShadowText(PowerText, Padding, Padding + 122.0f, Character && Character->HasThunderHammer() ? FLinearColor(1.0f, 0.84f, 0.25f) : FLinearColor(0.78f, 0.88f, 0.92f), 0.86f);

    if (Character && Character->GetProgressionComponent())
    {
        const ULightWarriorProgressionComponent* Progression = Character->GetProgressionComponent();
        DrawValuePill(TEXT("Light"), FString::Printf(TEXT("%.0f"), Progression->GetResource(ELightWarriorResource::Light)), Padding, Padding + 148.0f, 118.0f, FLinearColor(0.60f, 0.92f, 1.0f));
        DrawValuePill(TEXT("Courage"), FString::Printf(TEXT("%.0f"), Progression->GetResource(ELightWarriorResource::Courage)), Padding + 130.0f, Padding + 148.0f, 124.0f, FLinearColor(1.0f, 0.72f, 0.26f));

        const float ShadowDebt = Progression->GetResource(ELightWarriorResource::ShadowDebt);
        if (ShadowDebt > 0.0f)
        {
            DrawValuePill(TEXT("Debt"), FString::Printf(TEXT("%.0f"), ShadowDebt), Padding + 266.0f, Padding + 148.0f, 100.0f, FLinearColor(0.68f, 0.24f, 0.95f));
        }
    }

    DrawBar(TEXT("Active Well"), Padding, Padding + 182.0f, PanelWidth - 138.0f, 12.0f, GetBestLightWellProgress(), FLinearColor(0.47f, 0.83f, 1.0f));

    const float ObjectiveWidth = FMath::Clamp(ViewW * 0.38f, 390.0f, 620.0f);
    const float ObjectiveX = FMath::Clamp(ViewW - ObjectiveWidth - Padding, Padding, ViewW - ObjectiveWidth - Padding);
    DrawPanel(ObjectiveX, 24.0f, ObjectiveWidth, 76.0f, FLinearColor(0.012f, 0.018f, 0.030f, 0.66f));
    DrawShadowText(BuildObjectiveLine(GameMode), ObjectiveX + 20.0f, 42.0f, FLinearColor(0.94f, 0.98f, 1.0f), 0.96f);
    DrawShadowText(TEXT("Restore wells. Strike shadows. Leave safety when the temple calls."), ObjectiveX + 20.0f, 67.0f, FLinearColor(0.70f, 0.82f, 0.90f), 0.76f);

    const float HelpWidth = FMath::Clamp(ViewW * 0.52f, 560.0f, 780.0f);
    DrawPanel(Padding - 12.0f, ViewH - 70.0f, HelpWidth, 48.0f, FLinearColor(0.012f, 0.018f, 0.030f, 0.62f));
    DrawShadowText(TEXT("WASD move   Mouse look   Shift dash   Space/Left Click light strike"), Padding, ViewH - 56.0f, FLinearColor(0.78f, 0.86f, 0.90f), 0.76f);
    DrawShadowText(TEXT("Light wells restore the node. Thunder Hammer turns risk into a field surge."), Padding, ViewH - 34.0f, FLinearColor(0.96f, 0.82f, 0.42f), 0.72f);

    DrawCenterReticle();
}

FString ALightWarriorHUD::FormatRunState(const ALightWarriorGameMode* GameMode) const
{
    if (!GameMode)
    {
        return TEXT("RUN STATE UNKNOWN");
    }

    switch (GameMode->GetRunState())
    {
    case ELightWarriorRunState::Waiting:
        return TEXT("AWAITING IGNITION");
    case ELightWarriorRunState::Running:
        return TEXT("NODE RESTORATION ACTIVE");
    case ELightWarriorRunState::Victory:
        return TEXT("NODE RESTORED");
    case ELightWarriorRunState::Failure:
        return TEXT("FIELD COLLAPSED");
    default:
        return TEXT("NODE STATE UNKNOWN");
    }
}

FString ALightWarriorHUD::FormatTime(float Seconds) const
{
    const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(Seconds));
    return FString::Printf(TEXT("%02d:%02d"), TotalSeconds / 60, TotalSeconds % 60);
}

FString ALightWarriorHUD::BuildObjectiveLine(const ALightWarriorGameMode* GameMode) const
{
    if (!GameMode)
    {
        return TEXT("Reclaim the node");
    }

    switch (GameMode->GetRunState())
    {
    case ELightWarriorRunState::Victory:
        return TEXT("Restoration complete");
    case ELightWarriorRunState::Failure:
        return TEXT("Rise and reclaim the field");
    default:
        return FString::Printf(TEXT("Purify light wells: %d / %d"), GameMode->GetPurifiedLightWells(), GameMode->GetRequiredLightWells());
    }
}

float ALightWarriorHUD::GetBestLightWellProgress() const
{
    float BestWellProgress = 0.0f;
    for (TActorIterator<ALightWell> It(GetWorld()); It; ++It)
    {
        if (!It->IsPurified())
        {
            BestWellProgress = FMath::Max(BestWellProgress, It->GetPurification01());
        }
    }

    return BestWellProgress;
}

void ALightWarriorHUD::DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& Color) const
{
    FCanvasTileItem Panel(FVector2D(X, Y), FVector2D(Width, Height), Color);
    Panel.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Panel);
}

void ALightWarriorHUD::DrawBar(const FString& Label, float X, float Y, float Width, float Height, float Percent, const FLinearColor& FillColor) const
{
    DrawShadowText(Label, X, Y - 3.0f, FLinearColor(0.76f, 0.84f, 0.88f), 0.74f);

    FCanvasTileItem Backdrop(FVector2D(X + 92.0f, Y), FVector2D(Width, Height), FLinearColor(0.03f, 0.045f, 0.070f, 0.88f));
    Backdrop.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Backdrop);

    FCanvasTileItem Fill(FVector2D(X + 92.0f, Y), FVector2D(Width * FMath::Clamp(Percent, 0.0f, 1.0f), Height), FillColor);
    Fill.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Fill);
}

void ALightWarriorHUD::DrawValuePill(const FString& Label, const FString& Value, float X, float Y, float Width, const FLinearColor& AccentColor) const
{
    DrawPanel(X, Y, Width, 24.0f, FLinearColor(0.026f, 0.036f, 0.054f, 0.86f));
    DrawPanel(X, Y + 21.0f, Width, 3.0f, AccentColor);
    DrawShadowText(Label, X + 8.0f, Y + 5.0f, FLinearColor(0.68f, 0.76f, 0.80f), 0.64f);
    DrawShadowText(Value, X + Width - 44.0f, Y + 4.0f, FLinearColor(0.95f, 0.98f, 1.0f), 0.72f);
}

void ALightWarriorHUD::DrawCenterReticle() const
{
    const float CenterX = Canvas->ClipX * 0.5f;
    const float CenterY = Canvas->ClipY * 0.5f;
    const FLinearColor ReticleColor(0.90f, 0.98f, 1.0f, 0.56f);

    DrawPanel(CenterX - 14.0f, CenterY - 1.0f, 9.0f, 2.0f, ReticleColor);
    DrawPanel(CenterX + 5.0f, CenterY - 1.0f, 9.0f, 2.0f, ReticleColor);
    DrawPanel(CenterX - 1.0f, CenterY - 14.0f, 2.0f, 9.0f, ReticleColor);
    DrawPanel(CenterX - 1.0f, CenterY + 5.0f, 2.0f, 9.0f, ReticleColor);
}

void ALightWarriorHUD::DrawShadowText(const FString& Text, float X, float Y, const FLinearColor& Color, float Scale) const
{
    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    if (!Font)
    {
        return;
    }

    FCanvasTextItem Shadow(FVector2D(X + 1.0f, Y + 1.0f), FText::FromString(Text), Font, FLinearColor(0.0f, 0.0f, 0.0f, 0.78f));
    Shadow.Scale = FVector2D(Scale, Scale);
    Canvas->DrawItem(Shadow);

    FCanvasTextItem Foreground(FVector2D(X, Y), FText::FromString(Text), Font, Color);
    Foreground.Scale = FVector2D(Scale, Scale);
    Canvas->DrawItem(Foreground);
}
