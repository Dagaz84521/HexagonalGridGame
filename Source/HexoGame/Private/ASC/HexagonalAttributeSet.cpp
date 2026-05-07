// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC/HexagonalAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UHexagonalAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, Initiative, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, ActionPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, MaxActionPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, MovementRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, FistPalmProficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, SwordProficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, BladeProficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, QiangBangProficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, AnQiProficiency, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHexagonalAttributeSet, QiMengProficiency, COND_None, REPNOTIFY_Always);
}

void UHexagonalAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Health, OldHealth);
}

void UHexagonalAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, MaxHealth, OldMaxHealth);
}

void UHexagonalAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Shield, OldShield);
}

void UHexagonalAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, MaxShield, OldMaxShield);
}

void UHexagonalAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Mana, OldMana);
}

void UHexagonalAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, MaxMana, OldMaxMana);
}

void UHexagonalAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldAttack)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Attack, OldAttack);
}

void UHexagonalAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Defense, OldDefense);
}

void UHexagonalAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Agility, OldAgility);
}

void UHexagonalAttributeSet::OnRep_Initiative(const FGameplayAttributeData& OldInitiative)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, Initiative, OldInitiative);
}

void UHexagonalAttributeSet::OnRep_ActionPoints(const FGameplayAttributeData& OldActionPoints)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, ActionPoints, OldActionPoints);
}

void UHexagonalAttributeSet::OnRep_MaxActionPoints(const FGameplayAttributeData& OldMaxActionPoints)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, MaxActionPoints, OldMaxActionPoints);
}

void UHexagonalAttributeSet::OnRep_MovementRange(const FGameplayAttributeData& OldMovementRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, MovementRange, OldMovementRange);
}

void UHexagonalAttributeSet::OnRep_FistPalmProficiency(const FGameplayAttributeData& OldFistPalmProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, FistPalmProficiency, OldFistPalmProficiency);
}

void UHexagonalAttributeSet::OnRep_SwordProficiency(const FGameplayAttributeData& OldSwordProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, SwordProficiency, OldSwordProficiency);
}

void UHexagonalAttributeSet::OnRep_BladeProficiency(const FGameplayAttributeData& OldBladeProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, BladeProficiency, OldBladeProficiency);
}

void UHexagonalAttributeSet::OnRep_QiangBangProficiency(const FGameplayAttributeData& OldQiangBangProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, QiangBangProficiency, OldQiangBangProficiency);
}

void UHexagonalAttributeSet::OnRep_AnQiProficiency(const FGameplayAttributeData& AnQiWeaponProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, AnQiProficiency, AnQiWeaponProficiency);
}

void UHexagonalAttributeSet::OnRep_QiMengProficiency(const FGameplayAttributeData& OldQiMengProficiency)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHexagonalAttributeSet, QiMengProficiency, OldQiMengProficiency);
}
