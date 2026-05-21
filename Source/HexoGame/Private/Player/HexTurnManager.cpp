// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HexTurnManager.h"

#include "ASC/HexagonalAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Pawn/HexBattleUnit.h"

AHexTurnManager::AHexTurnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHexTurnManager::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoCollectUnitsOnBeginPlay)
	{
		CollectUnitsInWorld();
	}
}

void AHexTurnManager::RegisterUnit(AHexBattleUnit* Unit)
{
	if (!IsValid(Unit))
	{
		return;
	}

	RegisteredUnitObjects.AddUnique(Unit);
}

void AHexTurnManager::UnregisterUnit(AHexBattleUnit* Unit)
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
}

void AHexTurnManager::ClearRegisteredUnits()
{
	RegisteredUnitObjects.Reset();
	TurnQueue.Reset();
	CurrentUnit = nullptr;
	CurrentTurnIndex = INDEX_NONE;
	OnCurrentUnitChanged.Broadcast(nullptr);
}

void AHexTurnManager::CollectUnitsInWorld()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AHexBattleUnit::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		RegisterUnit(Cast<AHexBattleUnit>(Actor));
	}
}

void AHexTurnManager::BuildTurnQueue()
{
	TurnQueue.Reset();

	for (AHexBattleUnit* Unit : RegisteredUnitObjects)
	{
		if (!IsValid(Unit))
		{
			UE_LOG(LogTemp, Warning, TEXT("HexTurnManager skipped an invalid registered unit."));
			continue;
		}

		const UHexagonalAttributeSet* AttributeSet = Unit->AttributeSet;
		if (!AttributeSet)
		{
			UE_LOG(LogTemp, Warning, TEXT("HexTurnManager skipped unit %s because it has no HexagonalAttributeSet."), *GetNameSafe(Unit));
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

void AHexTurnManager::StartBattle()
{
	BuildTurnQueue();

	if (TurnQueue.Num() == 0)
	{
		CurrentTurnIndex = INDEX_NONE;
		CurrentUnit = nullptr;
		OnCurrentUnitChanged.Broadcast(nullptr);
		return;
	}

	CurrentTurnIndex = 0;
	BeginCurrentUnitTurn();
}

void AHexTurnManager::BeginCurrentUnitTurn()
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
}

void AHexTurnManager::EndCurrentUnitTurn()
{
	if (TurnQueue.Num() == 0)
	{
		CurrentTurnIndex = INDEX_NONE;
		CurrentUnit = nullptr;
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

