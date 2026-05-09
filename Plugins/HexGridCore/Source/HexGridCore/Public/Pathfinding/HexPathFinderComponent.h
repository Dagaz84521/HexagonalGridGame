// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/HexCoord.h"
#include "Components/ActorComponent.h"
#include "HexPathFinderComponent.generated.h"


class AHexGridMap;

struct FHexAStarNode
{
	FHexCoord Coord;
	int32 GCost;
	int32 FCost;

	FHexAStarNode() : GCost(0), FCost(0) {}
	FHexAStarNode(const FHexCoord& InCoord, int32 InGCost, int32 InFCost)
		: Coord(InCoord), GCost(InGCost), FCost(InFCost) {}
};

struct FMinHeapPredicate
{
	bool operator()(const FHexAStarNode& A, const FHexAStarNode& B) const
	{
		return A.FCost < B.FCost; // 小顶堆
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXGRIDCORE_API UHexPathFinderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHexPathFinderComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	TArray<FHexCoord> FindPath(const FHexCoord& Start, const FHexCoord& Goal) const;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AHexGridMap* Map;
};
