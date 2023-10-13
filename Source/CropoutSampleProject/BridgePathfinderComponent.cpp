// Fill out your copyright notice in the Description page of Project Settings.


#include "BridgePathfinderComponent.h"

#include "Engine/Internal/Kismet/BlueprintTypeConversions.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
// Sets default values for this component's properties

UBridgePathfinderComponent::UBridgePathfinderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts

void UBridgePathfinderComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogCore, Display, TEXT("Path blockers num on begin play: %d, for: %s"), PathBlockers.Num(), *this->GetName());
}

bool UBridgePathfinderComponent::FindAndSetAvailableConstructionPaths(const USceneComponent* SceneComponent,
	ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject,
	const int MaxOffsets, TArray<FVector>& OutFirstPath, FVector& PathDirection, FPossiblePaths& OutPossiblePaths)
{
	FindAndSetBridgePossiblePaths(SceneComponent, TraceTypeQuery, ActorsToIgnore, WorldContextObject, MaxOffsets);
	CurrentPathIndex = 0;
	OutFirstPath = TArray<FVector>();
	bool bIsAnyPathValid = false;
	
	if (AvailableDirections.IsEmpty())
	{
		return false;
	}
	for(auto PathsIt = PossibleConstructionPaths.CreateConstIterator(); PathsIt; ++PathsIt)
	{
		if(PathsIt.Value().Num() < 2)
		{
			continue;
		}
		bIsAnyPathValid = true;
		FPossiblePath PossiblePath;
		PossiblePath.PossiblePath = PathsIt.Value();
		OutPossiblePaths.PossibleConstructionPaths.Add(UKismetMathLibrary::FTruncVector(PathsIt.Key()), PossiblePath);
	}


	if (const auto Path = PossibleConstructionPaths.Find(AvailableDirections[0]))
	{
		OutFirstPath = *Path;
		PathDirection = AvailableDirections[0];//todo make int
		OnPathUpdated.Broadcast(OutFirstPath);//todo is it needed this?
	}
	
	return bIsAnyPathValid;
}

bool UBridgePathfinderComponent::GetNextPath(TArray<FVector>& NextPath, FVector& PathDirection)
{
	if(AvailableDirections.IsEmpty())
	{
		return false;
	}

	if(AvailableDirections.Num() == 1)
	{
		NextPath = *PossibleConstructionPaths.Find(AvailableDirections[0]);
		PathDirection = AvailableDirections[0];
		return false;
	}

	CurrentPathIndex++;
	if(AvailableDirections.IsValidIndex(CurrentPathIndex) == false)
	{
		CurrentPathIndex = 0;
	}

	if(AvailableDirections.IsValidIndex(CurrentPathIndex) == false)
	{
		UE_LOG(LogCore, Warning, TEXT("Available paths is not empty but index %d is invalid, this should not happen!"), CurrentPathIndex);
		return false;
	}

	NextPath = *PossibleConstructionPaths.Find(AvailableDirections[CurrentPathIndex]);
	PathDirection = AvailableDirections[CurrentPathIndex];//TODO: Access violation... Probably during recursion something writes to it. Implement differently. It can be turned into iterator of available directions, something calls GetNextPath but in the mean time something else proceeds with path query and modifies the directions
	return true;
}

bool UBridgePathfinderComponent::TryGetPathForActorRotation(AActor* Actor, const USceneComponent* SceneComponent,
	ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject,
	const int MaxOffsets, TArray<FVector>& OutFoundPath)
{
	FindAndSetBridgePossiblePaths(SceneComponent, TraceTypeQuery, ActorsToIgnore, WorldContextObject, MaxOffsets);

	const FVector PathDirection = Actor->GetActorForwardVector();
	const auto Key = AvailableDirections.FindLastByPredicate([&](const FVector& Direction)
	{
		return Direction.Equals(PathDirection, 0.01);
	});
	
	if(Key == INDEX_NONE)//todo: vector comparison should be integer cast
	{
		UE_LOG(LogCore, Error, TEXT("No match for actor rotation in available paths!"))
		return false;
	}

	OutFoundPath = *PossibleConstructionPaths.Find(AvailableDirections[Key]);
	return true;
}

void UBridgePathfinderComponent::FindAndSetBridgePossiblePaths(const USceneComponent* SceneComponent, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets = 10)
{
	AvailableDirections.Empty();
	for (auto It = PossibleConstructionPaths.CreateIterator(); It; ++It)
	{
		It.RemoveCurrent();
	}
	
	FVector Origin;
	FVector BoxExtent;
	float SphereRadius;
	UKismetSystemLibrary::GetComponentBounds(SceneComponent, Origin, BoxExtent, SphereRadius);

	//calculate four corners will be inside internal method;
	const float PositiveX = BoxExtent.X * 1.05f;
	const float NegativeX = BoxExtent.X * -1.05f;
	const float PositiveY = BoxExtent.Y * 1.05f;
	const float NegativeY = BoxExtent.Y * -1.05f;

	const FVector UpRightCornerOffset {PositiveX, PositiveY, 0};
	const FVector DownRightCornerOffset {PositiveX, NegativeY, 0};
	const FVector UpLeftCornerOffset {NegativeX, PositiveY, 0};
	const FVector DownLeftCornerOffset {NegativeX, NegativeY, 0};
	
	BridgeInNav(FVector::Zero(), Origin, BoxExtent.X, BoxExtent.Y, UpRightCornerOffset, DownRightCornerOffset, UpLeftCornerOffset, DownLeftCornerOffset, TraceTypeQuery, ActorsToIgnore, WorldContextObject, MaxOffsets);
}

//have to return vectors and the offset key to put them in a map
void UBridgePathfinderComponent::BridgeInNav(const FVector& OffsetDirectionKey, const FVector& Origin, const float BoxExtentX, const float BoxExtentY, const FVector& UpRightCornerOffset, const FVector& DownRightCornerOffset, const FVector& UpLeftCornerOffset, const FVector& DownLeftCornerOffset, ETraceTypeQuery TraceTypeQuery, const TArray<AActor*>& ActorsToIgnore, const UObject* WorldContextObject, const int MaxOffsets)
{
	const FVector UpRightCorner = UpRightCornerOffset + Origin;
	const FVector DownRightCorner = DownRightCornerOffset + Origin;
	const FVector UpLeftCorner = UpLeftCornerOffset + Origin;
	const FVector DownLeftCorner = DownLeftCornerOffset + Origin;

	static const FVector ZTraceStartOffset {0, 0, 100};
	static const FVector ZTraceEndOffset {0, 0, -1};

	TArray<FHitResult> HitResults;
	//todo maybe can be optimized to trace single?
	const bool bUpRightCorner = UKismetSystemLibrary::LineTraceMulti(WorldContextObject, UpRightCorner + ZTraceStartOffset, UpRightCorner + ZTraceEndOffset, TraceTypeQuery, false, ActorsToIgnore, CornersLineTraceType, HitResults, true);
	const bool bDownRightCorner = UKismetSystemLibrary::LineTraceMulti(WorldContextObject, DownRightCorner + ZTraceStartOffset, DownRightCorner + ZTraceEndOffset, TraceTypeQuery, false, ActorsToIgnore, CornersLineTraceType, HitResults, true);
	const bool bDownLeftCorner = UKismetSystemLibrary::LineTraceMulti(WorldContextObject, DownLeftCorner + ZTraceStartOffset, DownLeftCorner + ZTraceEndOffset, TraceTypeQuery, false, ActorsToIgnore, CornersLineTraceType, HitResults, true);
	const bool bUpLeftCorner = UKismetSystemLibrary::LineTraceMulti(WorldContextObject, UpLeftCorner + ZTraceStartOffset, UpLeftCorner+ ZTraceEndOffset, TraceTypeQuery, false, ActorsToIgnore, CornersLineTraceType, HitResults, true);
	
	// FHitResult HitResult;
	// UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Origin + ZTraceStartOffset, Origin + ZTraceEndOffset, TraceTypeQuery, false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);
	// HitResults.Add(HitResult);

	//todo add channel interactable
	UKismetSystemLibrary::BoxTraceMulti(WorldContextObject, Origin + ZTraceStartOffset, Origin + ZTraceEndOffset, FVector(BoxExtentX, BoxExtentY, BoxExtentX > BoxExtentY ? BoxExtentX : BoxExtentY),
		OffsetDirectionKey.ToOrientationRotator(), TraceTypeQuery, false, ActorsToIgnore, OverlappingActorsBoxTraceType, HitResults, true);//todo add margin around for space?
	//HitResults.Add(HitResult);

	for (auto HitsIt = HitResults.CreateConstIterator(); HitsIt; ++HitsIt)
	{
		UClass* HitResultClass = HitsIt->HitObjectHandle.GetRepresentedClass();

		if(HitResultClass == nullptr)
		{
			continue;
		}
		
		if (PathBlockers.ContainsByPredicate([&](const TSubclassOf<AActor> Blocker)
		{
			//auto IsInA = HitResultClass->IsInA(Blocker.Get());//todo alternative
			return Blocker.Get() == HitResultClass || HitResultClass->IsChildOf(Blocker.Get());
		}
		))//todo try IsA() instead but it works only on actors
		{
			if(AvailableDirections.Contains(OffsetDirectionKey))
			{
				AvailableDirections.Remove(OffsetDirectionKey);
				PossibleConstructionPaths.FindAndRemoveChecked(OffsetDirectionKey);
			}
			//UE_LOG(LogCore, Warning, TEXT("Overlaps listed subclass of name: %s and segment shouldn't be allowed to be placed!"), *HitResultClass->GetName());
			return;
		}
	}
	
	auto OffsetOrigin = [&]
	{
		const FVector ScaledOffset = FVector{BoxExtentX * 2, BoxExtentY * 2, 0} * OffsetDirectionKey;
		return Origin + ScaledOffset;
	};

	//if is start first position, meaning
	auto CanAnchorBridgeEnd = [&](const FVector& OffsetDirection)
	{
		bool CanAnchor = false;
		if(bUpRightCorner && bDownRightCorner)
		{
			CanAnchor |= FVector{1, 0, 0} == OffsetDirection;
		}

		if(bUpLeftCorner && bDownLeftCorner)
		{
			CanAnchor |= FVector{-1, 0, 0} == OffsetDirection;
		}

		if(bUpLeftCorner && bUpRightCorner)
		{
			CanAnchor |= FVector{0, 1, 0} == OffsetDirection;
		}

		if(bDownLeftCorner && bDownRightCorner)
		{
			CanAnchor |= FVector{0, -1, 0} == OffsetDirection;
		}

		return CanAnchor;
	};
	
	if (OffsetDirectionKey == FVector::Zero())//at least two must be grounded, it is the first location, todo maybe check also for obstacles at the entrance to the bridge?
	{
		//add first start element (assumes that we are placing it near the shore, but it can be expanded):
		TArray<FVector> OffsetDirections;
		if((bUpRightCorner && bDownRightCorner) == false && bUpLeftCorner && bDownLeftCorner)
		{
			OffsetDirections.Add(FVector{1, 0, 0});
		}

		if((bUpLeftCorner && bDownLeftCorner) == false && bUpRightCorner && bDownRightCorner)
		{
			OffsetDirections.Add(FVector{-1, 0, 0});
		}

		if((bUpLeftCorner && bUpRightCorner) == false && bDownLeftCorner && bDownRightCorner)
		{
			OffsetDirections.Add(FVector{0, 1, 0});
		}

		if((bDownLeftCorner && bDownRightCorner) == false && bUpLeftCorner && bUpRightCorner)
		{
			OffsetDirections.Add(FVector{0, -1, 0});
		}

		AvailableDirections = OffsetDirections;//todo eliminate directions that have an obstacle in front of the entrance? simpler solution is to add an empty mesh in front of both entrances, but then starting condition key must be different
		for (auto OffsetDirection : OffsetDirections)
		{
			TArray<FVector> PossiblePath = PossibleConstructionPaths.FindOrAdd(OffsetDirection);
			PossiblePath.Add(Origin);
			BridgeInNav(OffsetDirection, OffsetOrigin(), BoxExtentX, BoxExtentY, UpRightCornerOffset, DownRightCornerOffset, UpLeftCornerOffset, DownLeftCornerOffset, TraceTypeQuery, ActorsToIgnore, WorldContextObject, MaxOffsets);
		}
	}
	else
	{
		const auto PossiblePath = PossibleConstructionPaths.Find(OffsetDirectionKey);

		if (CanAnchorBridgeEnd(OffsetDirectionKey))
		{
			PossiblePath->Add(Origin);
			return;
		}

		if (PossiblePath->Num() < MaxOffsets)
		{
			PossiblePath->Add(Origin);
			BridgeInNav(OffsetDirectionKey, OffsetOrigin(), BoxExtentX, BoxExtentY, UpRightCornerOffset, DownRightCornerOffset, UpLeftCornerOffset, DownLeftCornerOffset, TraceTypeQuery, ActorsToIgnore, WorldContextObject, MaxOffsets);
		}
		else
		{
			PossibleConstructionPaths.FindAndRemoveChecked(OffsetDirectionKey);
		}
	}
}

