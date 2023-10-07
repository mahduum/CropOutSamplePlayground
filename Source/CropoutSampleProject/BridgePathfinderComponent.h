// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BridgePathfinderComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathUpdated, const TArray<FVector>&, CurrentPath);

UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CROPOUTSAMPLEPROJECT_API UBridgePathfinderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBridgePathfinderComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// todo make custom path class to store a path similar to like FNavigationPath is made
	UFUNCTION(BlueprintCallable, Category="Build Mode")
	UPARAM(DisplayName = "Can Be Placed") bool FindAndSetAvailableConstructionPaths(const USceneComponent* SceneComponent, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets, TArray<FVector>& OutFirstPath, FVector& PathDirection);

	UFUNCTION(BlueprintCallable)
	bool GetNextPath(TArray<FVector>& NextPath, FVector& PathDirection);

	UFUNCTION(BlueprintCallable)
	bool TryGetPathForActorRotation(AActor* Actor, const USceneComponent* SceneComponent, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets, TArray<FVector>& OutFoundPath);

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Build Mode")
	FOnPathUpdated OnPathUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build Mode")
	TArray<TSubclassOf<AActor>> PathBlockers;

	UPROPERTY(EditAnywhere, Category="Debug")
	TEnumAsByte<EDrawDebugTrace::Type> CornersLineTraceType;

	UPROPERTY(EditAnywhere, Category="Debug")
	TEnumAsByte<EDrawDebugTrace::Type> OverlappingActorsBoxTraceType;

	UFUNCTION(BlueprintCallable, Category="Build Mode")
	inline int AvailablePathsCount() const {return AvailableDirections.Num();}

	UFUNCTION(BlueprintGetter)
	inline int GetCurrentPathIndex() const {return CurrentPathIndex;};

private:
	TMap<FVector, TArray<FVector>> PossibleConstructionPaths;
	TArray<FVector> AvailableDirections;
	TArray<FVector> CurrentPath;

	UPROPERTY(BlueprintGetter = GetCurrentPathIndex)
	int CurrentPathIndex;

	void FindAndSetBridgePossiblePaths(const USceneComponent* SceneComponent, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets);
	void BridgeInNav(const FVector& OffsetDirectionKey, const FVector& Origin, const float BoxExtentX, const float BoxExtentY, const FVector& UpRightCornerOffset, const FVector& DownRightCornerOffset, const FVector& UpLeftCornerOffset, const FVector& DownLeftCornerOffset, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets = 10);

		
};
