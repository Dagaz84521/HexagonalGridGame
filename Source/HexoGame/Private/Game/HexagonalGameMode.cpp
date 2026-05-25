// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HexagonalGameMode.h"
#include "Pawn/HexBattleUnit.h"
#include "HexBattleWorldSettings.h"
#include "Player/HexagonalGamePlayerController.h"
#include "Subsystem/HexGridSubsystem.h"
#include "Subsystem/PawnManagerSubsystem.h"
#include "Subsystem/TurnManagerSubsystem.h"

void AHexagonalGameMode::InitialSubsystem()
{
	bGridSubsystemReady = false;
	bPawnManagerSubsystemReady = false;
	bTurnManagerSubsystemReady = false;

	if (AHexBattleWorldSettings* BattleSettings = Cast<AHexBattleWorldSettings>(GetWorld()->GetWorldSettings()))
	{
		if (UHexGridSubsystem* GridSubsystem = GetWorld()->GetSubsystem<UHexGridSubsystem>())
		{
			GridSubsystem->LoadFromDataAsset(BattleSettings->GridDataAsset);
			GridSubsystem->SetDecalMaterial(BattleSettings->DecalMaterial);
			GridSubsystem->BuildGridVisuals();
			GridSubsystem->HideAllDecals();
			bGridSubsystemReady = true;

			if (UPawnManagerSubsystem* PawnManagerSubsystem = GetWorld()->GetSubsystem<UPawnManagerSubsystem>())
			{
				PawnManagerSubsystem->SpawnUnitsOnGrid(GridSubsystem, TestBattleConfig, UnitClass);
				bPawnManagerSubsystemReady = true;

				if (UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
				{
					bTurnManagerSubsystemReady = TurnManagerSubsystem->PrepareBattle();
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HexagonalGameMode: HexPawnManagerSubsystem not found!"));
			}
		}
	}
}

void AHexagonalGameMode::BeginPlay()
{
	Super::BeginPlay();
	InitialSubsystem();
	TryStartBattle();
}

void AHexagonalGameMode::NotifyPlayerControllerReady(const AHexagonalGamePlayerController* PlayerController)
{
	if (PlayerController)
	{
		bPlayerControllerReady = true;
	}

	TryStartBattle();
}

void AHexagonalGameMode::TryStartBattle()
{
	if (!bPlayerControllerReady || !bGridSubsystemReady || !bPawnManagerSubsystemReady || !bTurnManagerSubsystemReady)
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("HexagonalGameMode: Battle is not ready. Player=%d Grid=%d PawnManager=%d TurnManager=%d"),
			bPlayerControllerReady,
			bGridSubsystemReady,
			bPawnManagerSubsystemReady,
			bTurnManagerSubsystemReady
		);
		return;
	}

	UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UTurnManagerSubsystem>() : nullptr;
	if (!TurnManagerSubsystem)
	{
		bTurnManagerSubsystemReady = false;
		UE_LOG(LogTemp, Warning, TEXT("HexagonalGameMode: TryStartBattle failed because TurnManagerSubsystem was not found."));
		return;
	}

	if (TurnManagerSubsystem->IsBattleStarted())
	{
		return;
	}

	if (!TurnManagerSubsystem->IsBattlePrepared())
	{
		bTurnManagerSubsystemReady = TurnManagerSubsystem->PrepareBattle();
		if (!bTurnManagerSubsystemReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("HexagonalGameMode: TryStartBattle failed because battle preparation failed."));
			return;
		}
	}

	TurnManagerSubsystem->StartBattle();
}
