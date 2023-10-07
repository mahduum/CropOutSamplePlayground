// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreUObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintHelpers.generated.h"

UENUM(BlueprintType)
enum EPathDirection
{
	Up,
	Down,
	Left,
	Right,
};

/**
 * 
 */
UCLASS()
class CROPOUTSAMPLEPROJECT_API UBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static bool SaveObjectToPackage(UObject* ObjectToSave, const FString PackagePath);

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static UObject* LoadObjectFromPackage(const FString InLongPackageName, const FString InLongFileName);

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	static UPackage* LoadPackageLocal(const FString InLongPackageName);
};
