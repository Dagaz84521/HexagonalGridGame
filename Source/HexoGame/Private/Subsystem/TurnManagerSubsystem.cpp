// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/TurnManagerSubsystem.h"

#include "ASC/HexagonalAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Pawn/HexBattleUnit.h"

void UTurnManagerSubsystem::Deinitialize()
{
	ClearRegisteredUnits();
	Super::Deinitialize();
}

void UTurnManagerSubsystem::RegisterUnit(AHexBattleUnit* Unit)
{
	if (!IsValid(Unit))
	{
		return;
	}

	RegisteredUnitObjects.AddUnique(Unit);
}

void UTurnManagerSubsystem::UnregisterUnit(AHexBattleUnit* Unit)
{
	if (!Unit)
	{
		return;
	}

	RegisteredUnitObjects.Remove(Unit);

	if (CurrentUnit == Unit)
	{
		CurrentUnit = nullptr;
		CurrentTurnIndex = INDEX_NONE;
		OnCurrentUnitChanged.Broadcast(nullptr);
	}

	BuildTurnQueue();
	bBattlePrepared = TurnQueue.Num() > 0;
}

void UTurnManagerSubsystem::ClearRegisteredUnits()
{
	RegisteredUnitObjects.Reset();
	TurnQueue.Reset();
	CurrentUnit = nullptr;
	CurrentTurnIndex = INDEX_NONE;
	bBattlePrepared = false;
	bBattleStarted = false;
	OnCurrentUnitChanged.Broadcast(nullptr);
}

void UTurnManagerSubsystem::CollectUnitsInWorld()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHexBattleUnit::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		RegisterUnit(Cast<AHexBattleUnit>(Actor));
	}
}

void UTurnManagerSubsystem::BuildTurnQueue()
{
	TurnQueue.Reset();

	for (AHexBattleUnit* Unit : RegisteredUnitObjects)
	{
		if (!IsValid(Unit))
		{
			UE_LOG(LogTemp, Warning, TEXT("TurnManagerSubsystem skipped an invalid registered unit."));
			continue;
		}

		const UHexagonalAttributeSet* AttributeSet = Unit->AttributeSet;
		if (!AttributeSet)
		{
			UE_LOG(LogTemp, Warning, TEXT("TurnManagerSubsystem skipped unit %s because it has no HexagonalAttributeSet."), *GetNameSafe(Unit));
			continue;
		}

		FHexTurnEntry Entry;
		Entry.Unit = Unit;
		Entry.Initiative = AttributeSet->GetInitiative();
		Entry.Agility = AttributeSet->GetAgility();
		TurnQueue.Add(Entry);
	}

	TurnQueue.Sort([](const FHexTurnEntry& Left, const FHexTurnEntry& Right)
	{
		if (!FMath::IsNearlyEqual(Left.Initiative, Right.Initiative))
		{
			return Left.Initiative > Right.Initiative;
		}

		return Left.Agility > Right.Agility;
	});
}

bool UTurnManagerSubsystem::PrepareBattle()
{
	BuildTurnQueue();
	CurrentTurnIndex = INDEX_NONE;
	CurrentUnit = nullptr;
	bBattleStarted = false;
	bBattlePrepared = TurnQueue.Num() > 0;

	if (!bBattlePrepared)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurnManagerSubsystem: PrepareBattle failed because turn queue is empty."));
	}

	return bBattlePrepared;
}

void UTurnManagerSubsystem::StartBattle()
{
	if (!bBattlePrepared)
	{
		PrepareBattle();
	}

	if (TurnQueue.Num() == 0)
	{
		CurrentTurnIndex = INDEX_NONE;
		CurrentUnit = nullptr;
		bBattleStarted = false;
		OnCurrentUnitChanged.Broadcast(nullptr);
		return;
	}

	CurrentTurnIndex = 0;
	bBattleStarted = true;
	BeginCurrentUnitTurn();
}

void UTurnManagerSubsystem::BeginCurrentUnitTurn()
{
	if (!TurnQueue.IsValidIndex(CurrentTurnIndex))
	{
		CurrentUnit = nullptr;
		OnCurrentUnitChanged.Broadcast(nullptr);
		return;
	}

	CurrentUnit = TurnQueue[CurrentTurnIndex].Unit;
	if (!IsValid(CurrentUnit))
	{
		EndCurrentUnitTurn();
		return;
	}

	OnCurrentUnitChanged.Broadcast(CurrentUnit);
	UE_LOG(LogTemp, Log, TEXT("Broadcast!"));
}

void UTurnManagerSubsystem::EndCurrentUnitTurn()
{
	if (TurnQueue.Num() == 0)
	{
		CurrentTurnIndex = INDEX_NONE;
		CurrentUnit = nullptr;
		bBattleStarted = false;
		OnCurrentUnitChanged.Broadcast(nullptr);
		return;
	}

	CurrentTurnIndex = TurnQueue.IsValidIndex(CurrentTurnIndex) ? CurrentTurnIndex + 1 : 0;

	if (!TurnQueue.IsValidIndex(CurrentTurnIndex))
	{
		BuildTurnQueue();
		CurrentTurnIndex = TurnQueue.Num() > 0 ? 0 : INDEX_NONE;
	}

	BeginCurrentUnitTurn();
}
