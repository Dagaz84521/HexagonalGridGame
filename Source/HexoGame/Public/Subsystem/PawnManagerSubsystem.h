// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PawnManagerSubsystem.generated.h"

class UHexGridSubsystem;
class UBattleConfigDataAsset;
class AHexBattleUnit;
struct FUnitConfig;
/**
 * 
 */
UCLASS()
class HEXOGAME_API UPawnManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void SpawnUnitsOnGrid(UHexGridSubsystem* GridSubsystem, TObjectPtr<UBattleConfigDataAsset> BattleSettings, TSubclassOf<AHexBattleUnit> UnitClass);

	void SpawnUnit(const FUnitConfig& UnitConfig, UHexGridSubsystem* GridSubsystem, TSubclassOf<AHexBattleUnit> UnitClass);

	const TArray<TObjectPtr<AHexBattleUnit>>& GetSpawnedUnits() const { return SpawnedUnits; }

private:
	UPROPERTY()
	TArray<TObjectPtr<AHexBattleUnit>> SpawnedUnits;
};
