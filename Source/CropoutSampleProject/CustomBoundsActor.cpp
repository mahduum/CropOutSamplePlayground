// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomBoundsActor.h"


// Sets default values
ACustomBoundsActor::ACustomBoundsActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ActiveBoundsActor = this;
}

// Called when the game starts or when spawned
void ACustomBoundsActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACustomBoundsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACustomBoundsActor::GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent,
	bool bIncludeFromChildActors) const
{
	ActiveBoundsActor->GetActorBounds(bOnlyCollidingComponents, Origin, BoxExtent, bIncludeFromChildActors);
}

