// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexCoord.h"
#include "GameFramework/PlayerController.h"
#include "HexPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class AHexGridMap;
class AHexGridGenerator;
class AHexUnitBase;
class UHexPathFinderComponent;

UENUM(BlueprintType)
enum class EHexInputState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	UnitSelected UMETA(DisplayName = "Unit Selected")
};
/**
 * 
 */
UCLASS()
class HEXGRIDCORE_API AHexPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	void SetupInputComponent() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* LeftClickAction;
private:
	void HandleLeftClick();
	void HandleIdleClick(const FHexCoord& ClickedCoord);
	void HandleUnitSelectedClick(const FHexCoord& ClickedCoord);
	void EnterIdleState();
	void EnterUnitSelectedState(AHexUnitBase* Unit);
	void ClearPreviewPath();
	void ShowPreviewPath(const FHexCoord& GoalCoord);

	UPROPERTY()
	AHexGridGenerator* GridGenerator;

	UPROPERTY()
	AHexGridMap* Map;

	UPROPERTY()
	UHexPathFinderComponent* PathFinder = nullptr;

	UPROPERTY()
	AHexUnitBase* SelectedUnit = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Hex|Input")
	EHexInputState InputState = EHexInputState::Idle;

	FHexCoord CurrentHoveredCoord;
	bool bHasHoveredCoord = false;
	TArray<FHexCoord> CurrentPath;
};
