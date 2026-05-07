// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/Ability/HexGameplayAbilityBase.h"

#include "ASC/HexagonalAbilitySystemComponent.h"
#include "ASC/HexagonalAttributeSet.h"
#include "Pawn/HexBattleUnit.h"

UHexGameplayAbilityBase::UHexGameplayAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AHexBattleUnit* UHexGameplayAbilityBase::GetHexBattleUnitFromActorInfo() const
{
	return GetHexBattleUnitFromActorInfo(GetCurrentActorInfo());
}

UHexagonalAbilitySystemComponent* UHexGameplayAbilityBase::GetHexAbilitySystemComponentFromActorInfo() const
{
	return GetHexAbilitySystemComponentFromActorInfo(GetCurrentActorInfo());
}

UHexagonalAttributeSet* UHexGameplayAbilityBase::GetHexAttributeSetFromActorInfo() const
{
	return GetHexAttributeSetFromActorInfo(GetCurrentActorInfo());
}

AHexBattleUnit* UHexGameplayAbilityBase::GetHexBattleUnitFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return nullptr;
	}

	return Cast<AHexBattleUnit>(ActorInfo->AvatarActor.Get());
}

UHexagonalAbilitySystemComponent* UHexGameplayAbilityBase::GetHexAbilitySystemComponentFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return nullptr;
	}

	return Cast<UHexagonalAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
}

UHexagonalAttributeSet* UHexGameplayAbilityBase::GetHexAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AHexBattleUnit* HexBattleUnit = GetHexBattleUnitFromActorInfo(ActorInfo);
	if (!HexBattleUnit)
	{
		return nullptr;
	}

	return Cast<UHexagonalAttributeSet>(HexBattleUnit->AttributeSet);
}
