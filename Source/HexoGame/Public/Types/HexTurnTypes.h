// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexTurnTypes.generated.h"

class AHexBattleUnit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHexTurnUnitChangedSignature, AHexBattleUnit*, CurrentUnit);

USTRUCT(BlueprintType)
struct FHexTurnEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	TObjectPtr<AHexBattleUnit> Unit = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	float Initiative = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	float Agility = 0.0f;
};
