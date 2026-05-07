// Fill out your copyright notice in the Description page of Project Settings.

#include "ASC/Ability/HexGameplaySkillAbility.h"

#include "ASC/HexagonalAttributeSet.h"

UHexGameplaySkillAbility::UHexGameplaySkillAbility()
{
}

bool UHexGameplaySkillAbility::HasEnoughMana() const
{
	const UHexagonalAttributeSet* HexAttributeSet = GetHexAttributeSetFromActorInfo();
	if (!HexAttributeSet)
	{
		return false;
	}

	return HexAttributeSet->GetMana() >= ManaCost;
}
