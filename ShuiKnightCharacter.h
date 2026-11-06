// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "PaperFlipbookComponent.h"
#include "ShuiKnightCharacter.generated.h"

UCLASS()
class SHUISKNIGHT_API AShuiKnightCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShuiKnightCharacter();


protected:

	void Move(const FInputActionValue& Value);
	void Jump();
	void StopJump();
	void Dash();
	void Attack();
	void SpawnSwordImpactEffect(FVector ImpactLocation);
	float DashSpeed = 1200.0f;
	float DashDuration = 0.2f;
	float DashTimeRemaining = 0.0f;
	bool bIsDashing = false;
	float DashDirection = 1.0f;
	float MoveSpeed = 300.0f;
	float JumpSpeed = 800.0f;
	float Gravity = 1800.0f;
	float VerticalVelocity = 0.0f;
	float PogoBounceForce = 850.0f;
	bool bHasPogodThisAttack = false;
	bool bIsGrounded = false;
	bool bIsAttacking = false;
	float AttackCooldown = 0.5f;
	float AttackCooldownRemaining = 0.0f;
	bool bIsUpAttacking = false;
	bool bIsDownAttacking = false;
	float AttackDamage = 25.0f;
	int32 JumpCount = 0;
	int32 MaxJumps = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;
	FVector CheckpointLocation = FVector::ZeroVector;
	bool bIsRespawning = false;
	float RespawnDelay = 1.0f;
	float RespawnTimer = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCapsuleComponent* CapsuleComponent;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPaperSpriteComponent* SpriteComponent;
	UPROPERTY(Transient)
	UPaperFlipbookComponent* AnimatedFlipbookComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Sprites")
	UPaperSprite* IdleSprite;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
	UPaperFlipbook* AttackFlipbook;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
	UPaperFlipbook* IdleFlipbook;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
	UPaperFlipbook* WalkFlipbook;
	// Called when the game starts or when spawned
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ReceiveDamage(float DamageAmount);
	void ActivateCheckpoint(FVector NewCheckpointLocation);
	

};
