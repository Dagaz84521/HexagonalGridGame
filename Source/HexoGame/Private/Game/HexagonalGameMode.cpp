// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/HexagonalGameMode.h"

#include "HexBattleWorldSettings.h"
#include "Subsystem/HexGridSubsystem.h"

void AHexagonalGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (AHexBattleWorldSettings* BattleSettings = Cast<AHexBattleWorldSettings>(GetWorld()->GetWorldSettings()))
	{
		if (UHexGridSubsystem* GridSubsystem = GetWorld()->GetSubsystem<UHexGridSubsystem>())
		{
			GridSubsystem->LoadFromDataAsset(BattleSettings->GridDataAsset);
			GridSubsystem->SetDecalMaterial(BattleSettings->DecalMaterial);
			GridSubsystem->BuildGridVisuals();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AHexagonalGameMode: Failed to get UHexGridSubsystem, cannot load grid data."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AHexagonalGameMode: WorldSettings is not AHexBattleWorldSettings, cannot load grid data."));
	}
}
