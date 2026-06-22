#include "LightWarriorCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SacredCircle.h"
#include "ShadowEnemy.h"

ALightWarriorCharacter::ALightWarriorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 620.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 620.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 900.0f;
    CameraBoom->SetRelativeRotation(FRotator(-58.0f, 0.0f, 0.0f));
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ALightWarriorCharacter::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

void ALightWarriorCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    DashCooldownRemaining = FMath::Max(0.0f, DashCooldownRemaining - DeltaSeconds);
    ThunderHammerTimer = FMath::Max(0.0f, ThunderHammerTimer - DeltaSeconds);
    ApplyCirclePressure(DeltaSeconds);
}

void ALightWarriorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ALightWarriorCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ALightWarriorCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ALightWarriorCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ALightWarriorCharacter::LookUp);
    PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &ALightWarriorCharacter::StartDash);
    PlayerInputComponent->BindAction(TEXT("LightStrike"), IE_Pressed, this, &ALightWarriorCharacter::LightStrike);
}

float ALightWarriorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Max(0.0f, Health - AppliedDamage);
    return AppliedDamage;
}

void ALightWarriorCharacter::GrantThunderHammer(float DurationSeconds)
{
    ThunderHammerTimer = FMath::Max(ThunderHammerTimer, DurationSeconds);
}

void ALightWarriorCharacter::MoveForward(float Value)
{
    if (Controller == nullptr || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ALightWarriorCharacter::MoveRight(float Value)
{
    if (Controller == nullptr || FMath::IsNearlyZero(Value))
    {
        return;
    }

    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void ALightWarriorCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ALightWarriorCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ALightWarriorCharacter::StartDash()
{
    if (DashCooldownRemaining > 0.0f)
    {
        return;
    }

    const FVector DashDirection = GetLastMovementInputVector().IsNearlyZero()
        ? GetActorForwardVector()
        : GetLastMovementInputVector().GetSafeNormal();

    LaunchCharacter(DashDirection * DashStrength, true, false);
    DashCooldownRemaining = DashCooldown;
}

void ALightWarriorCharacter::LightStrike()
{
    const float DamageAmount = HasThunderHammer() ? 80.0f : 35.0f;
    const float Radius = HasThunderHammer() ? StrikeRadius * 1.75f : StrikeRadius;
    bool bHitDarkness = false;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LightStrike), false, this);

    if (GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        Sphere,
        QueryParams))
    {
        for (const FOverlapResult& Result : Overlaps)
        {
            AShadowEnemy* Enemy = Cast<AShadowEnemy>(Result.GetActor());
            if (!Enemy)
            {
                continue;
            }

            UGameplayStatics::ApplyDamage(Enemy, DamageAmount, GetController(), this, UDamageType::StaticClass());
            bHitDarkness = true;
        }
    }

    if (bHitDarkness)
    {
        if (ASacredCircle* Circle = FindSacredCircle())
        {
            Circle->ExpandFromCombat(HasThunderHammer() ? StrikeExpansionAmount * 2.4f : StrikeExpansionAmount);
        }
    }
}

void ALightWarriorCharacter::ApplyCirclePressure(float DeltaSeconds)
{
    const ASacredCircle* Circle = FindSacredCircle();
    if (!Circle || Circle->IsInsideCircle(GetActorLocation()))
    {
        return;
    }

    Health = FMath::Max(0.0f, Health - OutsideCircleDamagePerSecond * DeltaSeconds);
}

ASacredCircle* ALightWarriorCharacter::FindSacredCircle() const
{
    for (TActorIterator<ASacredCircle> It(GetWorld()); It; ++It)
    {
        return *It;
    }

    return nullptr;
}
