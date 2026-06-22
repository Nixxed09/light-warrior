#include "LightWarriorProgressionComponent.h"

ULightWarriorProgressionComponent::ULightWarriorProgressionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void ULightWarriorProgressionComponent::AddResource(ELightWarriorResource Resource, float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    if (float* Value = ResolveResource(Resource))
    {
        *Value += Amount;
        BroadcastResource(Resource);
    }
}

bool ULightWarriorProgressionComponent::SpendResource(ELightWarriorResource Resource, float Amount)
{
    if (Amount <= 0.0f)
    {
        return true;
    }

    float* Value = ResolveResource(Resource);
    if (!Value || *Value < Amount)
    {
        return false;
    }

    *Value -= Amount;
    BroadcastResource(Resource);
    return true;
}

float ULightWarriorProgressionComponent::GetResource(ELightWarriorResource Resource) const
{
    if (const float* Value = ResolveResource(Resource))
    {
        return *Value;
    }

    return 0.0f;
}

void ULightWarriorProgressionComponent::RecordBraveExcursion()
{
    AddResource(ELightWarriorResource::Courage, 10.0f);
}

void ULightWarriorProgressionComponent::RecordRestoration(float LightAmount)
{
    AddResource(ELightWarriorResource::Light, LightAmount);
}

void ULightWarriorProgressionComponent::RecordPassivePressure(float DebtAmount)
{
    AddResource(ELightWarriorResource::ShadowDebt, DebtAmount);
}

float* ULightWarriorProgressionComponent::ResolveResource(ELightWarriorResource Resource)
{
    switch (Resource)
    {
    case ELightWarriorResource::Light:
        return &Light;
    case ELightWarriorResource::Courage:
        return &Courage;
    case ELightWarriorResource::ResonanceShards:
        return &ResonanceShards;
    case ELightWarriorResource::TrueMapFragments:
        return &TrueMapFragments;
    case ELightWarriorResource::ShadowDebt:
        return &ShadowDebt;
    case ELightWarriorResource::AetherCharge:
        return &AetherCharge;
    default:
        return nullptr;
    }
}

const float* ULightWarriorProgressionComponent::ResolveResource(ELightWarriorResource Resource) const
{
    return const_cast<ULightWarriorProgressionComponent*>(this)->ResolveResource(Resource);
}

void ULightWarriorProgressionComponent::BroadcastResource(ELightWarriorResource Resource)
{
    OnResourceChanged.Broadcast(Resource, GetResource(Resource));
}
