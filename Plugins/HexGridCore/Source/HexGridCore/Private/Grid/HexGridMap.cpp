#include "Grid/HexGridMap.h"

#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "Pathfinding/HexPathFinderComponent.h"
#include "Units/HexUnitBase.h"
#include "Materials/MaterialInstanceDynamic.h"

AHexGridMap::AHexGridMap()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	PathFinderComponent = CreateDefaultSubobject<UHexPathFinderComponent>(TEXT("PathFinderComponent"));
}

void AHexGridMap::BeginPlay()
{
	Super::BeginPlay();

	LoadFromDataAsset();
	BuildGridVisuals();
}

void AHexGridMap::LoadFromDataAsset()
{
	ClearGrid();

	if (!GridDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadFromDataAsset failed: GridDataAsset is not set."));
		return;
	}

	HexSize = GridDataAsset->HexSize;
	GridYaw = GridDataAsset->GridYaw;
	DecalYawOffset = GridDataAsset->DecalYawOffset;

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
		TEXT("LoadFromDataAsset loaded %d cells from %s."),
		GridData.Num(),
		*GridDataAsset->GetName()
	);
}

void AHexGridMap::BuildGridVisuals()
{
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
			UE_LOG(LogTemp, Warning, TEXT("BuildGridVisuals skipped: DecalMaterial is not set."));
		}
		return;
	}

	for (const TPair<FHexCoord, FHexCellData>& CellPair : GridData)
	{
		const FHexCellData& CellData = CellPair.Value;

		UDecalComponent* DecalComp = NewObject<UDecalComponent>(this);
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
		DecalComp->SetupAttachment(RootComponent);
		DecalComp->SetAbsolute(false, false, true);
		DecalComp->RegisterComponent();
		DecalComp->SetWorldLocation(CellData.WorldLocation);
		DecalComp->SetWorldRotation(FRotator(-90.0f, GridYaw + DecalYawOffset, 0.0f));

		SpawnedDecals.Add(DecalComp);
		DecalsMap.Add(CellPair.Key, DecalComp);
	}
}

void AHexGridMap::ClearGrid()
{
	GridData.Empty();

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

bool AHexGridMap::GetCellAtWorldLocation(const FVector& WorldLocation, FHexCoord& OutCoord) const
{
	if (GridData.IsEmpty() || HexSize <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float MaxDistanceSquared = FMath::Square(HexSize);
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FHexCoord* BestCoord = nullptr;

	for (const TPair<FHexCoord, FHexCellData>& CellPair : GridData)
	{
		const FVector CellLocation2D(CellPair.Value.WorldLocation.X, CellPair.Value.WorldLocation.Y, 0.0f);
		const FVector QueryLocation2D(WorldLocation.X, WorldLocation.Y, 0.0f);
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

bool AHexGridMap::GetCellData(const FHexCoord& Coord, FHexCellData& OutCellData) const
{
	const FHexCellData* CellData = GridData.Find(Coord);
	if (!CellData)
	{
		return false;
	}

	OutCellData = *CellData;
	return true;
}

bool AHexGridMap::SetCellDecalColor(const FHexCoord& Coord, const FLinearColor& Color)
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

bool AHexGridMap::SetCellDecalEnableCenterCircle(const FHexCoord& Coord, bool bEnable)
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

bool AHexGridMap::ResetCellDecalColor(const FHexCoord& Coord)
{
	return SetCellDecalColor(Coord, DefaultDecalColor);
}

bool AHexGridMap::IsValidCell(const FHexCoord& Coord) const
{
	return GridData.Contains(Coord);
}

bool AHexGridMap::IsCellOccupied(const FHexCoord& Coord) const
{
	return HexUnits.Contains(Coord);
}

bool AHexGridMap::CanCellMoveTo(const FHexCoord& Coord) const
{
	return IsValidCell(Coord) && !IsCellOccupied(Coord)  && GridData[Coord].bIsPassable;
}

bool AHexGridMap::CanMoveBetweenCells(const FHexCoord& FromCoord, const FHexCoord& ToCoord) const
{
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

AHexUnitBase* AHexGridMap::GetUnitAtCoord(const FHexCoord& Coord) const
{
	AHexUnitBase* const* UnitPtr = HexUnits.Find(Coord);
	return UnitPtr ? *UnitPtr : nullptr;
}

bool AHexGridMap::PlaceUnitAtCoord(AHexUnitBase* Unit, const FHexCoord& Coord)
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

bool AHexGridMap::MoveUnitToCoordNew(AHexUnitBase* Unit, const FHexCoord& TargetCoord)
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

	if (!PathFinderComponent)
	{
		return false;
	}

	const TArray<FHexCoord> PathCoords = PathFinderComponent->FindPath(*CurrentCoord, TargetCoord);
	if (PathCoords.Num() == 0)
	{
		return false;
	}

	TArray<FVector> PathPoints;
	TArray<FHexCoord> MovePathCoords;
	for (int32 Index = 0; Index < PathCoords.Num(); ++Index)
	{
		if (PathCoords[Index] == *CurrentCoord)
		{
			continue;
		}

		const FHexCellData* PathCellData = GridData.Find(PathCoords[Index]);
		if (!PathCellData)
		{
			return false;
		}

		PathPoints.Add(PathCellData->WorldLocation);
		MovePathCoords.Add(PathCoords[Index]);
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

bool AHexGridMap::MoveUnitToCoord(AHexUnitBase* Unit, const FHexCoord& TargetCoord)
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
