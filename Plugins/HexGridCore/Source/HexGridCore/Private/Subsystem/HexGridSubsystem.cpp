// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/HexGridSubsystem.h"

#include "Components/DecalComponent.h"
#include "Grid/HexGridDataAsset.h"
#include "Grid/HexGridMap.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Types/HexMathLibrary.h"
#include "Units/HexUnitBase.h"

namespace
{
	// Subsystem 版 A* 的临时节点，只在本实现文件内部使用。
	struct FHexSubsystemAStarNode
	{
		FHexCoord Coord;
		int32 GCost = 0;
		int32 FCost = 0;

		FHexSubsystemAStarNode() = default;

		FHexSubsystemAStarNode(const FHexCoord& InCoord, const int32 InGCost, const int32 InFCost)
			: Coord(InCoord)
			, GCost(InGCost)
			, FCost(InFCost)
		{
		}
	};

	struct FHexSubsystemMinHeapPredicate
	{
		bool operator()(const FHexSubsystemAStarNode& A, const FHexSubsystemAStarNode& B) const
		{
			return A.FCost < B.FCost;
		}
	};
}

void UHexGridSubsystem::Deinitialize()
{
	ClearGrid();
	Super::Deinitialize();
}

bool UHexGridSubsystem::LoadFromDataAsset(UHexGridDataAsset* InGridDataAsset)
{
	ClearGrid();

	GridDataAsset = InGridDataAsset;
	if (!GridDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("HexGridSubsystem LoadFromDataAsset failed: GridDataAsset is not set."));
		return false;
	}

	HexSize = GridDataAsset->HexSize;
	GridYaw = GridDataAsset->GridYaw;
	DecalYawOffset = GridDataAsset->DecalYawOffset;

	// DataAsset 使用 FVector2D 作为可序列化键，这里还原为运行时使用的 FHexCoord。
	for (const TPair<FVector2D, FHexCellData>& CellPair : GridDataAsset->GridData)
	{
		const FVector2D& CoordKey = CellPair.Key;
		const FHexCoord Coord(
			FMath::RoundToInt(CoordKey.X),
			FMath::RoundToInt(CoordKey.Y)
		);

		GridData.Add(Coord, CellPair.Value);
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("HexGridSubsystem loaded %d cells from %s."),
		GridData.Num(),
		*GridDataAsset->GetName()
	);
	return true;
}

void UHexGridSubsystem::ImportFromGridMap(AHexGridMap* GridMap)
{
	ClearGrid();

	if (!IsValid(GridMap))
	{
		UE_LOG(LogTemp, Warning, TEXT("HexGridSubsystem ImportFromGridMap failed: GridMap is invalid."));
		return;
	}

	// 只复制旧 Map 的地图配置和格子数据，不接管旧 Map 已经生成的贴花和组件。
	GridDataAsset = GridMap->GridDataAsset;
	HexSize = GridMap->HexSize;
	GridYaw = GridMap->GridYaw;
	GridData = GridMap->GridData;
	DecalMaterial = GridMap->DecalMaterial;
	DecalSize = GridMap->DecalSize;
	DecalYawOffset = GridMap->DecalYawOffset;
	MaxStepHeight = GridMap->MaxStepHeight;

	UE_LOG(LogTemp, Log, TEXT("HexGridSubsystem imported %d cells from %s."), GridData.Num(), *GridMap->GetName());
}

void UHexGridSubsystem::BuildGridVisuals()
{
	// 重新生成前先销毁 Subsystem 自己创建的贴花，避免重复叠加。
	for (UDecalComponent* Decal : SpawnedDecals)
	{
		if (IsValid(Decal))
		{
			Decal->DestroyComponent();
		}
	}

	SpawnedDecals.Empty();
	DecalsMap.Empty();

	if (!DecalMaterial)
	{
		if (GridData.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("HexGridSubsystem BuildGridVisuals skipped: DecalMaterial is not set."));
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TPair<FHexCoord, FHexCellData>& CellPair : GridData)
	{
		const FHexCellData& CellData = CellPair.Value;

		// Subsystem 没有 Actor Root，贴花直接注册到 World，并由 SpawnedDecals 负责生命周期。
		UDecalComponent* DecalComp = NewObject<UDecalComponent>(World);
		if (!DecalComp)
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, this);
		if (!DynamicMaterial)
		{
			continue;
		}

		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), DefaultDecalColor);
		DecalComp->SetDecalMaterial(DynamicMaterial);
		DecalComp->DecalSize = DecalSize;
		DecalComp->SetAbsolute(false, false, true);
		DecalComp->RegisterComponentWithWorld(World);
		DecalComp->SetWorldLocation(CellData.WorldLocation);
		DecalComp->SetWorldRotation(FRotator(-90.0f, GridYaw + DecalYawOffset, 0.0f));

		SpawnedDecals.Add(DecalComp);
		DecalsMap.Add(CellPair.Key, DecalComp);
	}
}

void UHexGridSubsystem::ClearGrid()
{
	// 这里清理的是 Subsystem 自己维护的运行时状态，不会触碰场景里的 AHexGridMap。
	GridData.Empty();
	HexUnits.Empty();

	for (UDecalComponent* Decal : SpawnedDecals)
	{
		if (IsValid(Decal))
		{
			Decal->DestroyComponent();
		}
	}

	SpawnedDecals.Empty();
	DecalsMap.Empty();
}

bool UHexGridSubsystem::GetCellAtWorldLocation(const FVector& WorldLocation, FHexCoord& OutCoord) const
{
	if (GridData.IsEmpty() || HexSize <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float MaxDistanceSquared = FMath::Square(HexSize);
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FHexCoord* BestCoord = nullptr;
	const FVector QueryLocation2D(WorldLocation.X, WorldLocation.Y, 0.0f);

	// 沿用旧 Map 的“最近中心点”策略，保证迁移期间点击判定行为一致。
	for (const TPair<FHexCoord, FHexCellData>& CellPair : GridData)
	{
		const FVector CellLocation2D(CellPair.Value.WorldLocation.X, CellPair.Value.WorldLocation.Y, 0.0f);
		const float DistanceSquared = FVector::DistSquared(CellLocation2D, QueryLocation2D);

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestCoord = &CellPair.Key;
		}
	}

	if (!BestCoord || BestDistanceSquared > MaxDistanceSquared)
	{
		return false;
	}

	OutCoord = *BestCoord;
	return true;
}

bool UHexGridSubsystem::GetCellData(const FHexCoord& Coord, FHexCellData& OutCellData) const
{
	const FHexCellData* CellData = GridData.Find(Coord);
	if (!CellData)
	{
		return false;
	}

	OutCellData = *CellData;
	return true;
}

bool UHexGridSubsystem::SetCellDecalColor(const FHexCoord& Coord, const FLinearColor& Color)
{
	UDecalComponent* const* Decal = DecalsMap.Find(Coord);
	if (!Decal || !IsValid(*Decal))
	{
		return false;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>((*Decal)->GetDecalMaterial());
	if (!DynamicMaterial)
	{
		DynamicMaterial = (*Decal)->CreateDynamicMaterialInstance();
	}

	if (!DynamicMaterial)
	{
		return false;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	return true;
}

bool UHexGridSubsystem::SetCellDecalEnableCenterCircle(const FHexCoord& Coord, bool bEnable)
{
	UDecalComponent* const* Decal = DecalsMap.Find(Coord);
	if (!Decal || !IsValid(*Decal))
	{
		return false;
	}

	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>((*Decal)->GetDecalMaterial());
	if (!DynamicMaterial)
	{
		DynamicMaterial = (*Decal)->CreateDynamicMaterialInstance();
	}

	if (!DynamicMaterial)
	{
		return false;
	}

	DynamicMaterial->SetScalarParameterValue(TEXT("EnableCenterCircle"), bEnable ? 1.0f : 0.0f);
	return true;
}

bool UHexGridSubsystem::ResetCellDecalColor(const FHexCoord& Coord)
{
	return SetCellDecalColor(Coord, DefaultDecalColor);
}

bool UHexGridSubsystem::IsValidCell(const FHexCoord& Coord) const
{
	return GridData.Contains(Coord);
}

bool UHexGridSubsystem::IsCellOccupied(const FHexCoord& Coord) const
{
	return HexUnits.Contains(Coord);
}

bool UHexGridSubsystem::CanCellMoveTo(const FHexCoord& Coord) const
{
	const FHexCellData* CellData = GridData.Find(Coord);
	return CellData && !IsCellOccupied(Coord) && CellData->bIsPassable;
}

bool UHexGridSubsystem::CanMoveBetweenCells(const FHexCoord& FromCoord, const FHexCoord& ToCoord) const
{
	// 目标格必须可进入；起点只要求是有效格，避免当前单位占用起点导致寻路失败。
	if (!IsValidCell(FromCoord) || !CanCellMoveTo(ToCoord))
	{
		return false;
	}

	const FHexCellData* FromCellData = GridData.Find(FromCoord);
	const FHexCellData* ToCellData = GridData.Find(ToCoord);
	if (!FromCellData || !ToCellData)
	{
		return false;
	}

	const float HeightDelta = FMath::Abs(FromCellData->WorldLocation.Z - ToCellData->WorldLocation.Z);
	return HeightDelta <= MaxStepHeight;
}

AHexUnitBase* UHexGridSubsystem::GetUnitAtCoord(const FHexCoord& Coord) const
{
	AHexUnitBase* const* UnitPtr = HexUnits.Find(Coord);
	return UnitPtr ? *UnitPtr : nullptr;
}

bool UHexGridSubsystem::PlaceUnitAtCoord(AHexUnitBase* Unit, const FHexCoord& Coord)
{
	if (!IsValid(Unit) || !IsValidCell(Coord) || IsCellOccupied(Coord))
	{
		return false;
	}

	const FHexCellData* CellData = GridData.Find(Coord);
	if (!CellData || !CellData->bIsPassable)
	{
		return false;
	}

	// 同一个单位不能同时占用多个格子。
	for (const TPair<FHexCoord, AHexUnitBase*>& Pair : HexUnits)
	{
		if (Pair.Value == Unit)
		{
			return false;
		}
	}

	HexUnits.Add(Coord, Unit);
	Unit->SetCurrentHexCoord(Coord);
	Unit->SetActorLocation(CellData->WorldLocation);

	return true;
}

bool UHexGridSubsystem::MoveUnitToCoord(AHexUnitBase* Unit, const FHexCoord& TargetCoord)
{
	if (!IsValid(Unit) || !IsValidCell(TargetCoord))
	{
		return false;
	}

	const FHexCellData* TargetCellData = GridData.Find(TargetCoord);
	if (!TargetCellData || !TargetCellData->bIsPassable)
	{
		return false;
	}

	const FHexCoord* CurrentCoord = nullptr;
	for (const TPair<FHexCoord, AHexUnitBase*>& Pair : HexUnits)
	{
		if (Pair.Value == Unit)
		{
			CurrentCoord = &Pair.Key;
			break;
		}
	}

	if (!CurrentCoord)
	{
		return false;
	}

	if (*CurrentCoord == TargetCoord)
	{
		return true;
	}

	AHexUnitBase* ExistingUnitAtTarget = GetUnitAtCoord(TargetCoord);
	if (IsValid(ExistingUnitAtTarget) && ExistingUnitAtTarget != Unit)
	{
		return false;
	}

	HexUnits.Remove(*CurrentCoord);
	HexUnits.Add(TargetCoord, Unit);
	Unit->SetCurrentHexCoord(TargetCoord);
	Unit->SetActorLocation(TargetCellData->WorldLocation);

	return true;
}

bool UHexGridSubsystem::MoveUnitToCoordWithPath(AHexUnitBase* Unit, const FHexCoord& TargetCoord)
{
	if (!IsValid(Unit) || !IsValidCell(TargetCoord) || Unit->IsMoving())
	{
		return false;
	}

	const FHexCellData* TargetCellData = GridData.Find(TargetCoord);
	if (!TargetCellData || !TargetCellData->bIsPassable)
	{
		return false;
	}

	// 先根据占用表定位单位当前格子，后续再用 Subsystem 自己的寻路结果移动。
	const FHexCoord* CurrentCoord = nullptr;
	for (const TPair<FHexCoord, AHexUnitBase*>& Pair : HexUnits)
	{
		if (Pair.Value == Unit)
		{
			CurrentCoord = &Pair.Key;
			break;
		}
	}

	if (!CurrentCoord)
	{
		return false;
	}

	if (*CurrentCoord == TargetCoord)
	{
		return true;
	}

	AHexUnitBase* ExistingUnitAtTarget = GetUnitAtCoord(TargetCoord);
	if (IsValid(ExistingUnitAtTarget) && ExistingUnitAtTarget != Unit)
	{
		return false;
	}

	const TArray<FHexCoord> PathCoords = FindPath(*CurrentCoord, TargetCoord);
	if (PathCoords.Num() == 0)
	{
		return false;
	}

	TArray<FVector> PathPoints;
	TArray<FHexCoord> MovePathCoords;
	// 单位移动组件需要世界坐标路径，同时保留格子路径用于朝向更新。
	for (const FHexCoord& PathCoord : PathCoords)
	{
		if (PathCoord == *CurrentCoord)
		{
			continue;
		}

		const FHexCellData* PathCellData = GridData.Find(PathCoord);
		if (!PathCellData)
		{
			return false;
		}

		PathPoints.Add(PathCellData->WorldLocation);
		MovePathCoords.Add(PathCoord);
	}

	if (PathPoints.Num() == 0)
	{
		return false;
	}

	HexUnits.Remove(*CurrentCoord);
	HexUnits.Add(TargetCoord, Unit);
	Unit->MoveAlongHexPath(*CurrentCoord, MovePathCoords, PathPoints);
	Unit->SetCurrentHexCoord(TargetCoord);

	return true;
}

TArray<FHexCoord> UHexGridSubsystem::FindPath(const FHexCoord& Start, const FHexCoord& Goal) const
{
	TArray<FHexCoord> Path;

	// 起点必须存在且可通行，终点还需要未被其他单位占用。
	FHexCellData StartCellData;
	if (!GetCellData(Start, StartCellData) || !StartCellData.bIsPassable)
	{
		return Path;
	}

	if (!CanCellMoveTo(Goal))
	{
		return Path;
	}

	if (Start == Goal)
	{
		return Path;
	}

	TArray<FHexSubsystemAStarNode> OpenHeap;
	TSet<FHexCoord> ClosedSet;
	TMap<FHexCoord, FHexCoord> CameFrom;
	TMap<FHexCoord, int32> GCostMap;

	// FCost = 已走代价 + 六边形距离启发值，和旧 UHexPathFinderComponent 保持一致。
	const int32 StartGCost = 0;
	const int32 StartFCost = UHexMathLibrary::GetDistance(Start, Goal);
	OpenHeap.HeapPush(FHexSubsystemAStarNode(Start, StartGCost, StartFCost), FHexSubsystemMinHeapPredicate());
	GCostMap.Add(Start, 0);

	while (OpenHeap.Num() > 0)
	{
		FHexSubsystemAStarNode CurrentNode;
		OpenHeap.HeapPop(CurrentNode, FHexSubsystemMinHeapPredicate());
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

		if (CurrentCoord == Goal)
		{
			// 从终点沿 CameFrom 回溯，再反转得到从起点到终点的路径。
			while (CameFrom.Contains(CurrentCoord))
			{
				Path.Add(CurrentCoord);
				CurrentCoord = CameFrom.FindChecked(CurrentCoord);
			}

			Algo::Reverse(Path);
			UE_LOG(LogTemp, Log, TEXT("HexGridSubsystem path found with length: %d"), Path.Num());
			return Path;
		}

		ClosedSet.Add(CurrentCoord);

		for (const FHexCoord& Neighbor : UHexMathLibrary::GetAllNeighbours(CurrentCoord))
		{
			if (ClosedSet.Contains(Neighbor))
			{
				continue;
			}

			if (!CanMoveBetweenCells(CurrentCoord, Neighbor))
			{
				continue;
			}

			const int32 TentativeGCost = *CurrentBestGCost + 1;
			const int32* ExistingNeighborGCost = GCostMap.Find(Neighbor);
			// 如果已经存在更短或相同成本的路线，就跳过当前邻居。
			if (ExistingNeighborGCost && TentativeGCost >= *ExistingNeighborGCost)
			{
				continue;
			}

			CameFrom.Add(Neighbor, CurrentCoord);
			GCostMap.Add(Neighbor, TentativeGCost);

			const int32 HCost = UHexMathLibrary::GetDistance(Neighbor, Goal);
			const int32 TotalFCost = TentativeGCost + HCost;
			OpenHeap.HeapPush(FHexSubsystemAStarNode(Neighbor, TentativeGCost, TotalFCost), FHexSubsystemMinHeapPredicate());
		}
	}

	return Path;
}
