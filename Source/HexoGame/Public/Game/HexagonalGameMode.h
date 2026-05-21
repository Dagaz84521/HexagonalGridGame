// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/BattleConfigDataAsset.h"
#include "HexagonalGameMode.generated.h"

class AHexagonalGamePlayerController;
class AHexBattleUnit;
class AHexBattleWorldSettings;
/**
 * 
 */
UCLASS()
class HEXOGAME_API AHexagonalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	void InitialSubsystem();
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hex|Battle")
	TObjectPtr<UBattleConfigDataAsset> TestBattleConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hex|Battle")
	TSubclassOf<AHexBattleUnit> UnitClass;

	void NotifyPlayerControllerReady(const AHexagonalGamePlayerController* PlayerController);

	void TryStartBattle();

private:
	bool bPlayerControllerReady = false;
	bool bGridSubsystemReady = false;
	bool bPawnManagerSubsystemReady = false;
	bool bTurnManagerSubsystemReady = false;
};
