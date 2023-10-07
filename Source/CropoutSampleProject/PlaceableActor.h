// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlaceableActor.generated.h"

UCLASS()
class CROPOUTSAMPLEPROJECT_API APlaceableActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlaceableActor();

	UPROPERTY(BlueprintReadOnly, Category="BuildMode")
	TSubclassOf<AActor> Builder;
};
