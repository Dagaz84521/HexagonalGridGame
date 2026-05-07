// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/Ability/HexGameplayAbilityBase.h"
#include "HexGameplayActionAbility.generated.h"

/**
 * Base class for board actions such as movement, waiting, guarding, and turning.
 *
 * Actions are tactical choices on the hex grid. They can spend action points or
 * change board state, but they should not consume the "one skill per turn" rule.
 */
UCLASS(Abstract, Blueprintable)
class HEXOGAME_API UHexGameplayActionAbility : public UHexGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UHexGameplayActionAbility();
};
