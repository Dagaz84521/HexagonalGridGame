// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "HexagonalAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class HEXOGAME_API UHexagonalAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UHexagonalAbilitySystemComponent();

	void AbilityActorInfoSet();

	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>> StartupAbilities);

	void ApplayEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level = 1.0f);

	bool HasMatchingGameplayTagExact(const FGameplayTag& Tag) const;
};
