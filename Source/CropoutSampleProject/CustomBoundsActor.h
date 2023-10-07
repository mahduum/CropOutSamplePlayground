// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomBoundsActor.generated.h"

UCLASS()
class CROPOUTSAMPLEPROJECT_API ACustomBoundsActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACustomBoundsActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	AActor* ActiveBoundsActor;
	
	virtual void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent, bool bIncludeFromChildActors = false) const override;
};
