// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/HexCoord.h"
#include "GameFramework/PlayerController.h"
#include "HexagonalGamePlayerController.generated.h"

class ACameraPawn;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class AHexGridMap;
class AHexGridGenerator;
class AHexUnitBase;
class UHexPathFinderComponent;

UENUM(BlueprintType)
enum class EHexagonalGameInputState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	UnitSelected UMETA(DisplayName = "Unit Selected")
};
/**
 * 
 */
UCLASS()
class HEXOGAME_API AHexagonalGamePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* LeftClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* FocusSelectedUnitAction;

	UPROPERTY()
	ACameraPawn* CameraPawn = nullptr;

	void FocusCameraOnSelectedUnit();
	void UpdateEdgeScroll(float DeltaSeconds);
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
	EHexagonalGameInputState InputState = EHexagonalGameInputState::Idle;

	FHexCoord CurrentHoveredCoord;
	bool bHasHoveredCoord = false;
	TArray<FHexCoord> CurrentPath;

public:
	UPROPERTY(EditAnywhere, Category = "Camera")
	float EdgeScrollMargin = 200.f;
	UPROPERTY(EditAnywhere, Category = "Camera")
	float EdgeScrollSpeed = 1600.f;
};
