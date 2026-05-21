// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/HexTurnTypes.h"
#include "HexTurnManager.generated.h"

class AHexBattleUnit;

UCLASS()
class HEXOGAME_API AHexTurnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AHexTurnManager();

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
	void StartBattle();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void BeginCurrentUnitTurn();

	UFUNCTION(BlueprintCallable, Category = "Hex|Turn")
	void EndCurrentUnitTurn();

	UFUNCTION(BlueprintPure, Category = "Hex|Turn")
	AHexBattleUnit* GetCurrentUnit() const { return CurrentUnit; }

	UPROPERTY(BlueprintAssignable, Category = "Hex|Turn")
	FHexTurnUnitChangedSignature OnCurrentUnitChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Turn")
	bool bAutoCollectUnitsOnBeginPlay = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	TArray<TObjectPtr<AHexBattleUnit>> RegisteredUnitObjects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|Turn")
	TArray<FHexTurnEntry> TurnQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|Turn")
	TObjectPtr<AHexBattleUnit> CurrentUnit;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	int32 CurrentTurnIndex = INDEX_NONE;
};
