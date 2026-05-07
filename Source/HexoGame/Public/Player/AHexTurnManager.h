// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AHexTurnManager.generated.h"

class AHexBattleUnit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHexTurnUnitChangedSignature, AHexBattleUnit*, CurrentUnit);

USTRUCT(BlueprintType)
struct FHexTurnEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	TObjectPtr<AHexBattleUnit> Unit = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	float Initiative = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hex|Turn")
	float Agility = 0.0f;
};

UCLASS()
class HEXOGAME_API AAHexTurnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AAHexTurnManager();

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
