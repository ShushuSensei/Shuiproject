// Fill out your copyright notice in the Description page of Project Settings.


#include "ShuiEnemy.h"
#include "ShuiKnightCharacter.h"

// Sets default values
AShuiEnemy::AShuiEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentHealth = MaxHealth;
}

// Called when the game starts or when spawned
void AShuiEnemy::BeginPlay()
{
    Super::BeginPlay();

    PlayerPawn = GetWorld()->GetFirstPlayerController()
        ? GetWorld()->GetFirstPlayerController()->GetPawn()
		: nullptr;

    PatrolStartX = GetActorLocation().X;

    EnemyVisualComponent = FindComponentByClass<UPaperFlipbookComponent>();

    if (!EnemyVisualComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENEMY FLIPBOOK NOT FOUND!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ENEMY FLIPBOOK FOUND!"));
    }

    if (EnemyVisualComponent)
    {
        OriginalColor = EnemyVisualComponent->GetSpriteColor();
    }
}

// Called every frame
void AShuiEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
    }

    if (HitStunRemaining > 0.0f)
    {
        HitStunRemaining -= DeltaTime;

        AddActorWorldOffset(
            FVector(
                KnockbackVelocityX * DeltaTime,
                0.0f,
                KnockbackVelocityZ * DeltaTime
            ),
            true
        );

        KnockbackVelocityX = FMath::FInterpTo(
            KnockbackVelocityX, 0.0f, DeltaTime, 5.0f
        );

        KnockbackVelocityZ = FMath::FInterpTo(
            KnockbackVelocityZ, 0.0f, DeltaTime, 5.0f
        );
    }

    if (FlashRemaining > 0.0f)
    {
        FlashRemaining -= DeltaTime;

        if (FlashRemaining <= 0.0f && EnemyVisualComponent)
        {
            EnemyVisualComponent->SetSpriteColor(OriginalColor);
        }
    }

    if (HitStunRemaining <= 0.0f)
    {
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(
                GetActorLocation(),
                PlayerPawn->GetActorLocation()
            );

            bPlayerDetected = DistanceToPlayer <= DetectionRange;
        }

        if (bPlayerDetected && PlayerPawn)
        {
            float DistanceX = FMath::Abs(
                PlayerPawn->GetActorLocation().X - GetActorLocation().X
            );

            if (DistanceX > AttackRange)
            {
                float DirectionToPlayer =
                    PlayerPawn->GetActorLocation().X > GetActorLocation().X ? 1.0f : -1.0f;

                AddActorWorldOffset(
                    FVector(DirectionToPlayer * PatrolSpeed * 1.5f * DeltaTime, 0.0f, 0.0f),
                    true
                );
            }
            else
            {
                if (AttackCooldownRemaining <= 0.0f)
                {
                    AShuiKnightCharacter* Knight = Cast<AShuiKnightCharacter>(PlayerPawn);

                    if (Knight)
                    {
                        Knight->ReceiveDamage(AttackDamage);
                    }

                    AttackCooldownRemaining = AttackCooldown;
                }
            }
        }
        else
        {
            float TargetX = PatrolStartX + (PatrolDirection * PatrolDistance);

            if (FMath::Abs(GetActorLocation().X - TargetX) < 10.0f)
            {
                PatrolDirection *= -1.0f;
            }

            AddActorWorldOffset(
                FVector(PatrolDirection * PatrolSpeed * DeltaTime, 0.0f, 0.0f),
                true
            );
        }
    }
}


void AShuiEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AShuiEnemy::ReceiveDamage(float DamageAmount, float DirectionX, float DirectionZ)
{
    CurrentHealth -= DamageAmount;

    if (CurrentHealth <= 0.0f)
    {
        Destroy();
        return;
    }

    KnockbackVelocityX = DirectionX * KnockbackForce;
    KnockbackVelocityZ = DirectionZ * KnockbackForce;

    HitStunRemaining = HitStunDuration;
    FlashRemaining = FlashDuration;
    UE_LOG(LogTemp, Warning, TEXT("ENEMY FLASH ACTIVATED!"));

    
    if (EnemyVisualComponent)
    {
        EnemyVisualComponent->SetSpriteColor(FLinearColor::Red);
    }
}