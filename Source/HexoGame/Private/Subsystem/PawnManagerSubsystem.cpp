// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/PawnManagerSubsystem.h"
#include "HexBattleWorldSettings.h"
#include "Data/BattleConfigDataAsset.h"
#include "Pawn/HexBattleUnit.h"
#include "Subsystem/HexGridSubsystem.h"
#include "Subsystem/TurnManagerSubsystem.h"

void UPawnManagerSubsystem::SpawnUnitsOnGrid(UHexGridSubsystem* GridSubsystem,
                                             TObjectPtr<UBattleConfigDataAsset> BattleSettings, TSubclassOf<AHexBattleUnit> UnitClass)
{
	if (!BattleSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: BattleSettings is null, no units will be spawned."));
		return;
	}

	if (!UnitClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: UnitClass is null, no units will be spawned."));
		return;
	}

	UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (!TurnManagerSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManagerSubsystem not found!"));
		return;
	}

	TurnManagerSubsystem->ClearRegisteredUnits();

	// Spawn player units
	for (const FUnitConfig& SpawnInfo : BattleSettings->UnitConfigs)
	{
		SpawnUnit(SpawnInfo, GridSubsystem, UnitClass);
	}
}

void UPawnManagerSubsystem::SpawnUnit(const FUnitConfig& UnitConfig, UHexGridSubsystem* GridSubsystem, TSubclassOf<AHexBattleUnit> UnitClass)
{
	if (!UnitClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: SpawnUnit failed, UnitClass is null."));
		return;
	}

	if (!GridSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: SpawnUnit failed, GridSubsystem is null."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: SpawnUnit failed, World is null."));
		return;
	}

	FHexCellData CellData;
	if (!GridSubsystem->GetCellData(UnitConfig.HexCoord, CellData))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PawnManagerSubsystem: SpawnUnit failed, invalid spawn coord Q:%d R:%d."),
			UnitConfig.HexCoord.Q,
			UnitConfig.HexCoord.R
		);
		return;
	}

	if (!CellData.bIsPassable || GridSubsystem->IsCellOccupied(UnitConfig.HexCoord))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PawnManagerSubsystem: SpawnUnit failed, coord Q:%d R:%d is blocked or occupied."),
			UnitConfig.HexCoord.Q,
			UnitConfig.HexCoord.R
		);
		return;
	}

	const FRotator SpawnRotation = UHexMathLibrary::GetDirectionRotation(UnitConfig.FacingDirection, -90.0f);
	const FTransform SpawnTransform(SpawnRotation, CellData.WorldLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHexBattleUnit* SpawnedUnit = World->SpawnActor<AHexBattleUnit>(
		UnitClass,
		SpawnTransform,
		SpawnParams
	);

	if (!IsValid(SpawnedUnit))
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnManagerSubsystem: SpawnUnit failed, SpawnActor returned null."));
		return;
	}

	SpawnedUnit->SetFaction(UnitConfig.Faction);
	SpawnedUnit->SetFacingDirection(UnitConfig.FacingDirection);

	if (UnitConfig.UnitDefinition)
	{
		SpawnedUnit->InitializeFromUnitDefinition(UnitConfig.UnitDefinition);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PawnManagerSubsystem: Unit at Q:%d R:%d has no UnitDefinition, spawned with default mesh."),
			UnitConfig.HexCoord.Q,
			UnitConfig.HexCoord.R
		);
	}

	if (!GridSubsystem->PlaceUnitAtCoord(SpawnedUnit, UnitConfig.HexCoord))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PawnManagerSubsystem: SpawnUnit spawned %s but failed to place it at Q:%d R:%d."),
			*GetNameSafe(SpawnedUnit),
			UnitConfig.HexCoord.Q,
			UnitConfig.HexCoord.R
		);
		SpawnedUnit->Destroy();
		return;
	}

	SpawnedUnits.Add(SpawnedUnit);

	if (UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		TurnManagerSubsystem->RegisterUnit(SpawnedUnit);
	}
}
