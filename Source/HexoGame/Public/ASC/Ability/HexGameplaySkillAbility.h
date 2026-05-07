// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASC/Ability/HexGameplayAbilityBase.h"
#include "HexGameplaySkillAbility.generated.h"

/**
 * Base class for martial skills.
 *
 * Skills are the abilities that spend Mana/Neili and should later obey the
 * "one skill per turn" rule. Concrete skill abilities can inherit this in C++
 * or Blueprint.
 */
UCLASS(Abstract, Blueprintable)
class HEXOGAME_API UHexGameplaySkillAbility : public UHexGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UHexGameplaySkillAbility();

	UFUNCTION(BlueprintPure, Category = "Hex|Skill")
	float GetManaCost() const { return ManaCost; }

	UFUNCTION(BlueprintPure, Category = "Hex|Skill")
	bool HasEnoughMana() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hex|Skill|Cost", meta = (ClampMin = "0.0"))
	float ManaCost = 0.0f;
};
