// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Units/HexUnitBase.h"
#include "AbilitySystemInterface.h"
#include "HexBattleUnit.generated.h"

class UGameplayEffect;
class UHexagonalAttributeSet;
class UBattleUnitDefinition;
/**
 * 
 */
UCLASS()
class HEXOGAME_API AHexBattleUnit : public AHexUnitBase, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AHexBattleUnit();

	virtual void BeginPlay() override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UHexagonalAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> InitialAttributeEffect;

	void InitializeFromUnitDefinition(const UBattleUnitDefinition* UnitDefinition);

	int32 GetMoveRange() const;
	
private:
	void EnsureAbilitySystemInitialized();
	void ApplyInitialGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UPROPERTY(Transient)
	bool bAbilitySystemInitialized = false;
};
