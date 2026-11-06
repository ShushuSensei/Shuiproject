// Fill out your copyright notice in the Description page of Project Settings.


#include "ShuiCheckpoint.h"
#include "ShuiKnightCharacter.h"

// Sets default values
AShuiCheckpoint::AShuiCheckpoint()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    SetRootComponent(TriggerSphere);

    TriggerSphere->SetSphereRadius(100.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AShuiCheckpoint::OnTriggerOverlap
    );

}

// Called when the game starts or when spawned
void AShuiCheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShuiCheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AShuiCheckpoint::OnTriggerOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    AShuiKnightCharacter* Knight = Cast<AShuiKnightCharacter>(OtherActor);

    if (Knight)
    {
        Knight->ActivateCheckpoint(GetActorLocation());

        UE_LOG(LogTemp, Warning, TEXT("CHECKPOINT ACTIVATED!"));
    }
}

