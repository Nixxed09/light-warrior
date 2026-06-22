#include "ShadowEnemy.h"

#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorCharacter.h"

AShadowEnemy::AShadowEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCharacterMovement()->MaxWalkSpeed = 360.0f;
}

void AShadowEnemy::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

void AShadowEnemy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaSeconds);

    ALightWarriorCharacter* Player = FindPlayer();
    if (!Player)
    {
        return;
    }

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size2D();

    if (Distance > AttackRange)
    {
        AddMovementInput(ToPlayer.GetSafeNormal2D(), 1.0f);
        return;
    }

    if (AttackCooldownRemaining <= 0.0f)
    {
        UGameplayStatics::ApplyDamage(Player, ContactDamage, GetController(), this, UDamageType::StaticClass());
        AttackCooldownRemaining = AttackCooldown;
    }
}

float AShadowEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health -= AppliedDamage;

    if (Health <= 0.0f)
    {
        Destroy();
    }

    return AppliedDamage;
}

ALightWarriorCharacter* AShadowEnemy::FindPlayer() const
{
    return Cast<ALightWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}
