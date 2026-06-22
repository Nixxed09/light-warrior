#include "ShadowEnemy.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LightWarriorCharacter.h"

AShadowEnemy::AShadowEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCharacterMovement()->MaxWalkSpeed = 285.0f;

    ShadowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowMesh"));
    ShadowMesh->SetupAttachment(RootComponent);
    ShadowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -25.0f));
    ShadowMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
    ShadowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShadowMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (ShadowMeshAsset.Succeeded())
    {
        ShadowMesh->SetStaticMesh(ShadowMeshAsset.Object);
    }

    ShadowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShadowLight"));
    ShadowLight->SetupAttachment(RootComponent);
    ShadowLight->SetLightColor(FLinearColor(0.18f, 0.04f, 0.34f));
    ShadowLight->SetIntensity(1600.0f);
    ShadowLight->SetAttenuationRadius(520.0f);

    ShadowLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ShadowLabel"));
    ShadowLabel->SetupAttachment(RootComponent);
    ShadowLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
    ShadowLabel->SetHorizontalAlignment(EHTA_Center);
    ShadowLabel->SetTextRenderColor(FColor(198, 82, 255));
    ShadowLabel->SetWorldSize(58.0f);
    ShadowLabel->SetText(FText::FromString(TEXT("SHADOW")));
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
