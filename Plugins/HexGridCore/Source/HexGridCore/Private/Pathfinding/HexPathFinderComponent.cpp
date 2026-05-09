// Fill out your copyright notice in the Description page of Project Settings.


#include "Pathfinding/HexPathFinderComponent.h"
#include "Grid/HexGridMap.h"
#include "Types/HexMathLibrary.h"
#include "Grid/HexGridMap.h"



UHexPathFinderComponent::UHexPathFinderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}



void UHexPathFinderComponent::BeginPlay()
{
	Super::BeginPlay();
	Map = Cast<AHexGridMap>(GetOwner());
}


void UHexPathFinderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TArray<FHexCoord> UHexPathFinderComponent::FindPath(const FHexCoord& Start, const FHexCoord& Goal) const
{
	TArray<FHexCoord> Path;
	if (!Map)
	{
		return Path;
	}

	FHexCellData StartCellData;
	if (!Map->GetCellData(Start, StartCellData) || !StartCellData.bIsPassable)
	{
		return Path;
	}

	if (!Map->CanCellMoveTo(Goal))
	{
		return Path;
	}
	if (Start == Goal)
	{
		return Path;
	}
	TArray<FHexAStarNode> OpenHeap;
	TSet<FHexCoord> ClosedSet;
	TMap<FHexCoord, FHexCoord> CameFrom;
	TMap<FHexCoord, int32> GCostMap;

	const int32 StartGCost = 0;
	const int32 StartFCost = UHexMathLibrary::GetDistance(Start, Goal);
	OpenHeap.HeapPush(FHexAStarNode(Start, StartGCost, StartFCost), FMinHeapPredicate());
	GCostMap.Add(Start, 0);
	while (OpenHeap.Num() > 0)
	{
		FHexAStarNode CurrentNode;
		OpenHeap.HeapPop(CurrentNode, FMinHeapPredicate());
		FHexCoord CurrentCoord = CurrentNode.Coord;
		const int32* CurrentBestGCost = GCostMap.Find(CurrentCoord);
		if (!CurrentBestGCost)
		{
			continue;
		}
		if (CurrentNode.GCost != *CurrentBestGCost)
		{
			continue;
		}
		if (ClosedSet.Contains(CurrentCoord))
		{
			continue;
		}

		if (CurrentCoord == Goal) // 找到路径
		{
			while (CameFrom.Contains(CurrentCoord))
			{
				Path.Add(CurrentCoord);
				CurrentCoord = CameFrom.FindChecked(CurrentCoord);
			}
			Algo::Reverse(Path);
			UE_LOG(LogTemp, Log, TEXT("Path found with length: %d"), Path.Num());
			return Path;
		}
		ClosedSet.Add(CurrentCoord);
		
		for (const FHexCoord& Neighbor : UHexMathLibrary::GetAllNeighbours(CurrentCoord))
		{
			if (ClosedSet.Contains(Neighbor))
				continue;
			if (!Map->CanMoveBetweenCells(CurrentCoord, Neighbor))
				continue;
			const int32 TentativeGCost = *CurrentBestGCost + 1;
			const int32* ExistingNeighborGCost = GCostMap.Find(Neighbor);
			if (ExistingNeighborGCost && TentativeGCost >= *ExistingNeighborGCost)
			{
				continue;
			}

			CameFrom.Add(Neighbor, CurrentCoord);
			GCostMap.Add(Neighbor, TentativeGCost);

			const int32 HCost = UHexMathLibrary::GetDistance(Neighbor, Goal);
			const int32 TotalFCost = TentativeGCost + HCost;
			OpenHeap.HeapPush(FHexAStarNode(Neighbor, TentativeGCost, TotalFCost), FMinHeapPredicate());
		}
	}
	return Path;
}
