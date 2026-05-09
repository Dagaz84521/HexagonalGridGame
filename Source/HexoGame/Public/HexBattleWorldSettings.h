// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "Grid/HexGridDataAsset.h"
#include "HexBattleWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class HEXOGAME_API AHexBattleWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex|Grid")
	TObjectPtr<UHexGridDataAsset> GridDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex|Grid")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex|Grid")
	FVector DecalSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex|Grid")
	float MaxStepHeight = 100.0f;
};
