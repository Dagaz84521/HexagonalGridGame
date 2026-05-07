// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HexGameplayAbilityBase.generated.h"

class AHexBattleUnit;
class UHexagonalAbilitySystemComponent;
class UHexagonalAttributeSet;

/**
 * Base class for all HexoGame gameplay abilities.
 *
 * Keep this layer focused on shared accessors and common GAS defaults.
 * Movement, waiting, and skills should add their own rules in derived classes.
 */
UCLASS(Abstract, Blueprintable)
class HEXOGAME_API UHexGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHexGameplayAbilityBase();

protected:
	UFUNCTION(BlueprintPure, Category = "Hex|Ability")
	AHexBattleUnit* GetHexBattleUnitFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Hex|Ability")
	UHexagonalAbilitySystemComponent* GetHexAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Hex|Ability")
	UHexagonalAttributeSet* GetHexAttributeSetFromActorInfo() const;

	AHexBattleUnit* GetHexBattleUnitFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UHexagonalAbilitySystemComponent* GetHexAbilitySystemComponentFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UHexagonalAttributeSet* GetHexAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;
};
