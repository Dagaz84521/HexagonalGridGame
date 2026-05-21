// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleUnitDefinition.h"
#include "Engine/DataAsset.h"
#include "Types/HexCoord.h"
#include "Types/HexMathLibrary.h"
#include "Types/HexUnitTypes.h"
#include "BattleConfigDataAsset.generated.h"

/**
 * 
 */
USTRUCT()
struct FUnitConfig
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly)
	FHexCoord HexCoord;
	UPROPERTY(EditDefaultsOnly)
	EHexUnitFaction Faction;
	UPROPERTY(EditDefaultsOnly)
	EHexDirection FacingDirection;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBattleUnitDefinition> UnitDefinition;
};

UCLASS()
class HEXOGAME_API UBattleConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FUnitConfig> UnitConfigs;
};
