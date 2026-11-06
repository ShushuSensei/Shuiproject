// Fill out your copyright notice in the Description page of Project Settings.

#include "ShuiKnightCharacter.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"
#include "Engine/OverlapResult.h"
#include "ShuiEnemy.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "PaperFlipbookComponent.h"
#include "Engine/LocalPlayer.h"

// Sets default values
AShuiKnightCharacter::AShuiKnightCharacter()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetCapsuleHalfHeight(48.0f);
	CapsuleComponent->SetCapsuleRadius(20.0f);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(CapsuleComponent);
	
	
}
void AShuiKnightCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IdleSprite && SpriteComponent)
	{
		SpriteComponent->SetSprite(IdleSprite);
	}
}

void AShuiKnightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AShuiKnightCharacter::Move
		);

		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&AShuiKnightCharacter::Jump
		);
		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&AShuiKnightCharacter::StopJump
		);
		EnhancedInputComponent->BindAction(
			DashAction,
			ETriggerEvent::Started,
			this,
			&AShuiKnightCharacter::Dash
		);
		EnhancedInputComponent->BindAction(
			AttackAction,
			ETriggerEvent::Started,
			this,
			&AShuiKnightCharacter::Attack
		);
	}
}
void AShuiKnightCharacter::Move(const FInputActionValue& Value)
{
	if (bIsRespawning)
	{
		return;
	}
	const float MoveValue = Value.Get<float>();

	if (!FMath::IsNearlyZero(MoveValue) && AnimatedFlipbookComponent)
	{
		FVector CurrentScale = AnimatedFlipbookComponent->GetRelativeScale3D();
		CurrentScale.X = (MoveValue > 0.0f) ? 1.0f : -1.0f;
		AnimatedFlipbookComponent->SetRelativeScale3D(CurrentScale);

		if (!bIsAttacking && WalkFlipbook &&
			AnimatedFlipbookComponent->GetFlipbook() != WalkFlipbook)
		{
			AnimatedFlipbookComponent->SetFlipbook(WalkFlipbook);
			AnimatedFlipbookComponent->PlayFromStart();
		}

		AddActorWorldOffset(
			FVector(MoveValue * MoveSpeed * GetWorld()->GetDeltaSeconds(), 0.0f, 0.0f),
			true
		);
	}
}

void AShuiKnightCharacter::Jump()
{
	if (JumpCount < MaxJumps)
	{
		JumpCount++;
		bIsGrounded = false;
		VerticalVelocity = JumpSpeed;
	}
}

void AShuiKnightCharacter::StopJump()
{
	if (VerticalVelocity > 0.0f)
	{
		VerticalVelocity *= 0.5f;
	}
}

void AShuiKnightCharacter::Dash()
{
	if (bIsDashing)
	{
		return;
	}

	bIsDashing = true;
	DashTimeRemaining = DashDuration;
	DashDirection = (AnimatedFlipbookComponent &&
		AnimatedFlipbookComponent->GetRelativeScale3D().X < 0.0f) ? -1.0f : 1.0f;
}
void AShuiKnightCharacter::Attack()
{
	if (bIsAttacking || AttackCooldownRemaining > 0.0f)
	{
		return;
	}

	if (AnimatedFlipbookComponent && AttackFlipbook)
	{
		FVector AttackOffset = FVector::ZeroVector;
		APlayerController* PC = Cast<APlayerController>(GetController());

		bIsUpAttacking = PC && PC->IsInputKeyDown(EKeys::W);
		bIsDownAttacking = !bIsUpAttacking && PC && PC->IsInputKeyDown(EKeys::S);
		
		bIsAttacking = true;
		bHasPogodThisAttack = false;
		AttackCooldownRemaining = AttackCooldown;

		AnimatedFlipbookComponent->SetFlipbook(AttackFlipbook);
		AnimatedFlipbookComponent->PlayFromStart();

		// Determine which direction the knight is facing
		float FacingDirection =
			AnimatedFlipbookComponent->GetRelativeScale3D().X < 0.0f ? -1.0f : 1.0f;

		// Create a damage area in front of the knight
		if (bIsUpAttacking)
		{
			AttackOffset = FVector(0.0f, 0.0f, 90.0f);
		}
		else if (bIsDownAttacking)
		{
			AttackOffset = FVector(0.0f, 0.0f, -90.0f);
		}
		else
		{
			AttackOffset = FVector(FacingDirection * 90.0f, 0.0f, 0.0f);
		}

		FVector AttackCenter = GetActorLocation() + AttackOffset;

		TArray<FOverlapResult> Overlaps;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			AttackCenter,
			FQuat::Identity,
			ObjectParams,
			FCollisionShape::MakeSphere(120.0f),
			Params
		);

		// Damage every enemy inside the area
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (AShuiEnemy* Enemy = Cast<AShuiEnemy>(Overlap.GetActor()))
			{
				float KnockbackX = 0.0f;
				float KnockbackZ = 0.0f;
				bool bCanPogo = bIsDownAttacking && VerticalVelocity < 0.0f;

				if (bIsUpAttacking)
				{
					KnockbackZ = 1.0f;
				}
				else if (bIsDownAttacking)
				{
					KnockbackZ = -1.0f;
				}
				else
				{
					KnockbackX =
						Enemy->GetActorLocation().X >= GetActorLocation().X ? 1.0f : -1.0f;
				}

				Enemy->ReceiveDamage(AttackDamage, KnockbackX, KnockbackZ);
				if (bCanPogo && !bHasPogodThisAttack)
				{
					VerticalVelocity = PogoBounceForce;
					bIsGrounded = false;
					JumpCount = 0;

					bHasPogodThisAttack = true;
				}
				SpawnSwordImpactEffect(Enemy->GetActorLocation());
			}
		}
	}
}
void AShuiKnightCharacter::SpawnSwordImpactEffect(FVector ImpactLocation)
{
	for (int32 i = 0; i < 6; i++)
	{
		float Angle = (360.0f / 6.0f) * i;

		FVector Direction(
			FMath::Cos(FMath::DegreesToRadians(Angle)),
			0.0f,
			FMath::Sin(FMath::DegreesToRadians(Angle))
		);

		DrawDebugLine(
			GetWorld(),
			ImpactLocation,
			ImpactLocation + Direction * 35.0f,
			FColor::Orange,
			false,
			0.15f,
			0,
			3.0f
		);
	}

	DrawDebugSphere(
		GetWorld(),
		ImpactLocation,
		8.0f,
		8,
		FColor::Yellow,
		false,
		0.15f
	);
}



// Called when the game starts or when spawned
void AShuiKnightCharacter::BeginPlay()
{
	Super::BeginPlay();

	CheckpointLocation = GetActorLocation();
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	AnimatedFlipbookComponent = FindComponentByClass<UPaperFlipbookComponent>();

	if (PlayerController)
	{
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

		if (LocalPlayer)
		{
			UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

			if (Subsystem && DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}


	}
}

// Called every frame
void AShuiKnightCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRespawning)
	{
		RespawnTimer -= DeltaTime;

		if (RespawnTimer <= 0.0f)
		{
			SetActorLocation(CheckpointLocation);
			CurrentHealth = MaxHealth;
			VerticalVelocity = 0.0f;
			bIsGrounded = false;
			JumpCount = 0;
			bIsRespawning = false;
		}

		return;
	}

	if (AttackCooldownRemaining > 0.0f)
	
	{
		AttackCooldownRemaining -= DeltaTime;

		if (AttackCooldownRemaining <= 0.0f)
		{
			AttackCooldownRemaining = 0.0f;
			bIsAttacking = false;

			if (AnimatedFlipbookComponent)
			{
				if (!FMath::IsNearlyZero(GetLastMovementInputVector().X) && WalkFlipbook)
				{
					AnimatedFlipbookComponent->SetFlipbook(WalkFlipbook);
				}
				else if (IdleFlipbook)
				{
					AnimatedFlipbookComponent->SetFlipbook(IdleFlipbook);
				}

				AnimatedFlipbookComponent->PlayFromStart();
			}
		}
	}

	if (bIsDashing)
	{
		AddActorWorldOffset(
			FVector(DashDirection * DashSpeed * DeltaTime, 0.0f, 0.0f),
			true
		);

		DashTimeRemaining -= DeltaTime;

		if (DashTimeRemaining <= 0.0f)
		{
			bIsDashing = false;
		}
	}

	if (VerticalVelocity <= 0.0f)
	{
		bIsGrounded = GetWorld()->LineTraceTestByChannel(
			GetActorLocation(),
			GetActorLocation() - FVector(0.0f, 0.0f, 65.0f),
			ECC_WorldStatic,
			FCollisionQueryParams(FName(TEXT("GroundCheck")), false, this)
			);
	}

	
	

	if (!bIsGrounded)
	{
		VerticalVelocity -= Gravity * DeltaTime;
	}

	FHitResult Hit;

	AddActorWorldOffset(
		FVector(0.0f, 0.0f, VerticalVelocity * DeltaTime),
		true,
		&Hit
	);

	if (Hit.IsValidBlockingHit())
	{
		if (VerticalVelocity < 0.0f)
		{
			bIsGrounded = true;
			JumpCount = 0;
		}

		VerticalVelocity = 0.0f;
	}
}
void AShuiKnightCharacter::ReceiveDamage(float DamageAmount)
{
	if (bIsRespawning)
	{
		return;
	}

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		bIsRespawning = true;
		RespawnTimer = RespawnDelay;

		UE_LOG(LogTemp, Warning, TEXT("KNIGHT DOWN! RESPAWNING..."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("KNIGHT HEALTH: %f"), CurrentHealth);
}
void AShuiKnightCharacter::ActivateCheckpoint(FVector NewCheckpointLocation)
{
	CheckpointLocation = NewCheckpointLocation;

	UE_LOG(LogTemp, Warning, TEXT("NEW CHECKPOINT SAVED!"));
}


	




