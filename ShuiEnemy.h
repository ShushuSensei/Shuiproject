// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperFlipbookComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShuiEnemy.generated.h"

UCLASS()
class SHUISKNIGHT_API AShuiEnemy : public APawn
{
	GENERATED_BODY()

public:
	
	AShuiEnemy();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	UPROPERTY()
	UPaperFlipbookComponent* EnemyVisualComponent = nullptr;

	FLinearColor OriginalColor = FLinearColor::White;
	float FlashDuration = 0.25f;
	float FlashRemaining = 0.0f;
	
	virtual void BeginPlay() override;

public:	
	
	virtual void Tick(float DeltaTime) override;

	void ReceiveDamage(float DamageAmount, float DirectionX, float DirectionZ);

	float KnockbackForce = 500.0f;
	float KnockbackVelocityX = 0.0f;
	float KnockbackVelocityZ = 0.0f;
	float HitStunDuration = 0.25f;
	float HitStunRemaining = 0.0f;
	
	float PatrolSpeed = 100.0f;
	float PatrolDistance = 300.0f;
	float PatrolStartX = 0.0f;
	float PatrolDirection = 1.0f;
	float DetectionRange = 500.0f;
	float AttackRange = 100.0f;
	float AttackDamage = 15.0f;
	float AttackCooldown = 1.0f;
	float AttackCooldownRemaining = 0.0f;
	bool bPlayerDetetced = false;
	APawn* PlayerPawn = nullptr;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
