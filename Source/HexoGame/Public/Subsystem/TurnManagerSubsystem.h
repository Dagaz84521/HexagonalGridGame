// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/HexTurnTypes.h"
#include "TurnManagerSubsystem.generated.h"

class AHexBattleUnit;

UCLASS()
class HEXOGAME_API UTurnManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void RegisterUnit(AHexBattleUnit* Unit);

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void UnregisterUnit(AHexBattleUnit* Unit);

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void ClearRegisteredUnits();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void CollectUnitsInWorld();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void BuildTurnQueue();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	bool PrepareBattle();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void StartBattle();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void BeginCurrentUnitTurn();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void EndCurrentUnitTurn();

	UFUNCTION(BlueprintPure, Category = "Hex|Turn")
	AHexBattleUnit* GetCurrentUnit() const { return CurrentUnit; }

	UFUNCTION(BlueprintPure, Category = "Hex|Turn")
	const TArray<FHexTurnEntry>& GetTurnQueue() const { return TurnQueue; }

	UFUNCTION(BlueprintPure, Category = "Hex|Turn")
	bool IsBattlePrepared() const { return bBattlePrepared; }

	UFUNCTION(BlueprintPure, Category = "Hex|Turn")
	bool IsBattleStarted() const { return bBattleStarted; }

	UPROPERTY(BlueprintAssignable, Category = "Hex|Turn")
	FHexTurnUnitChangedSignature OnCurrentUnitChanged;

private:
	UPROPERTY()
	TArray<TObjectPtr<AHexBattleUnit>> RegisteredUnitObjects;

	UPROPERTY()
	TArray<FHexTurnEntry> TurnQueue;

	UPROPERTY()
	TObjectPtr<AHexBattleUnit> CurrentUnit;

	int32 CurrentTurnIndex = INDEX_NONE;

	bool bBattlePrepared = false;

	bool bBattleStarted = false;
	
};
