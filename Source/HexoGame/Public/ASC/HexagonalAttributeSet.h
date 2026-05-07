// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HexagonalAttributeSet.generated.h"




/**
 * 
 */
UCLASS()
class HEXOGAME_API UHexagonalAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, MaxShield)

	UPROPERTY(BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, MaxMana)

	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Attack)
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Attack);
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Defense);

	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Agility)
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Agility);

	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Initiative)
	FGameplayAttributeData Initiative;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, Initiative);

	UPROPERTY(BlueprintReadOnly, Category = "Action", ReplicatedUsing = OnRep_ActionPoints)
	FGameplayAttributeData ActionPoints;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, ActionPoints);

	UPROPERTY(BlueprintReadOnly, Category = "Action", ReplicatedUsing = OnRep_MaxActionPoints)
	FGameplayAttributeData MaxActionPoints;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, MaxActionPoints);

	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MovementRange)
	FGameplayAttributeData MovementRange;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, MovementRange);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_FistPalmProficiency)
	FGameplayAttributeData FistPalmProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, FistPalmProficiency);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_SwordProficiency)
	FGameplayAttributeData SwordProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, SwordProficiency);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_BladeProficiency)
	FGameplayAttributeData BladeProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, BladeProficiency);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_QiangBangProficiency)
	FGameplayAttributeData QiangBangProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, QiangBangProficiency);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_AnQiProficiency)
	FGameplayAttributeData AnQiProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, AnQiProficiency);

	UPROPERTY(BlueprintReadOnly, Category = "WeaponProficiency", ReplicatedUsing = OnRep_QiMengProficiency)
	FGameplayAttributeData QiMengProficiency;
	ATTRIBUTE_ACCESSORS(UHexagonalAttributeSet, QiMengProficiency);

	// RepNotifies
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Shield(const FGameplayAttributeData& OldShield);

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	virtual void OnRep_Attack(const FGameplayAttributeData& OldAttack);

	UFUNCTION()
	virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	UFUNCTION()
	virtual void OnRep_Agility(const FGameplayAttributeData& OldAgility);

	UFUNCTION()
	virtual void OnRep_Initiative(const FGameplayAttributeData& OldInitiative);

	UFUNCTION()
	virtual void OnRep_ActionPoints(const FGameplayAttributeData& OldActionPoints);

	UFUNCTION()
	virtual void OnRep_MaxActionPoints(const FGameplayAttributeData& OldMaxActionPoints);

	UFUNCTION()
	virtual void OnRep_MovementRange(const FGameplayAttributeData& OldMovementRange);

	UFUNCTION()
	virtual void OnRep_FistPalmProficiency(const FGameplayAttributeData& OldFistPalmProficiency);

	UFUNCTION()
	virtual void OnRep_SwordProficiency(const FGameplayAttributeData& OldSwordProficiency);

	UFUNCTION()
	virtual void OnRep_BladeProficiency(const FGameplayAttributeData& OldBladeProficiency);

	UFUNCTION()
	virtual void OnRep_QiangBangProficiency(const FGameplayAttributeData& OldQiangBangProficiency);

	UFUNCTION()
	virtual void OnRep_AnQiProficiency(const FGameplayAttributeData& AnQiWeaponProficiency);

	UFUNCTION()
	virtual void OnRep_QiMengProficiency(const FGameplayAttributeData& OldQiMengProficiency);

	
};
