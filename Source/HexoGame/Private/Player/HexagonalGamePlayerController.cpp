// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/HexagonalGamePlayerController.h"
#include "Game/HexagonalGameMode.h"
#include "Pawn/HexBattleUnit.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Player/CameraPawn.h"
#include "Subsystem/HexGridSubsystem.h"
#include "Subsystem/PawnManagerSubsystem.h"
#include "Subsystem/TurnManagerSubsystem.h"

void AHexagonalGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (TurnManagerSubsystem)
	{
		TurnManagerSubsystem->OnCurrentUnitChanged.AddDynamic(this, &AHexagonalGamePlayerController::OnCurrentUnitChanged);
		UE_LOG(LogTemp, Log, TEXT("PlayerController: Subscribed to OnCurrentUnitChanged event."));
	}

	if (AHexagonalGameMode* HexGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AHexagonalGameMode>() : nullptr)
	{
		HexGameMode->NotifyPlayerControllerReady(this);
	}
	
	bShowMouseCursor = true;
}

void AHexagonalGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(TestAction, ETriggerEvent::Started, this, &AHexagonalGamePlayerController::TestCommandFunction);
		EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AHexagonalGamePlayerController::HandleCameraZoom);
		EnhancedInputComponent->BindAction(CameraOrbitHoldAction, ETriggerEvent::Started, this, &AHexagonalGamePlayerController::StartCameraOrbit);
		EnhancedInputComponent->BindAction(CameraOrbitHoldAction, ETriggerEvent::Completed, this, &AHexagonalGamePlayerController::EndCameraOrbit);
		EnhancedInputComponent->BindAction(CameraOrbitHoldAction, ETriggerEvent::Canceled, this, &AHexagonalGamePlayerController::EndCameraOrbit);
		EnhancedInputComponent->BindAction(CameraOrbitAction, ETriggerEvent::Triggered, this, &AHexagonalGamePlayerController::HandleCameraOrbit);
	}
}

void AHexagonalGamePlayerController::UpdatePathPreview()
{
	if (!bHoveredCell || CurrentMovementRangeCells.Find(CurrentHoveredCell) == INDEX_NONE)
	{
		ClearPathPreview();
		return;
	}
	UHexGridSubsystem* HexGridSubsystem = GetWorld()->GetSubsystem<UHexGridSubsystem>();
	UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>();
	if (!HexGridSubsystem || !TurnManagerSubsystem)	{
		ClearPathPreview();
		return;
	}
	AHexBattleUnit* CurrentUnit = TurnManagerSubsystem->GetCurrentUnit();
	if (!IsValid(CurrentUnit))
	{
		ClearPathPreview();
		return;
	}

	const TArray<FHexCoord> NewPathPreview = HexGridSubsystem->FindPath(CurrentUnit->GetCurrentHexCoord(), CurrentHoveredCell);
	if (NewPathPreview == CurrentPathPreview)
	{
		return;
	}

	ClearPathPreview();
	CurrentPathPreview = NewPathPreview;

	for (const FHexCoord& Coord : CurrentPathPreview)
	{
		HexGridSubsystem->SetCellDecalColor(Coord, PathPreviewColor);
		HexGridSubsystem->SetDecalVisible(Coord, true);
	}

	RestoreCellVisual(CurrentHoveredCell);
}

void AHexagonalGamePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleEdgeScroll(DeltaTime);
	UpdateHoveredCell();
	UpdatePathPreview();
}

void AHexagonalGamePlayerController::SimulateEnemyForTest()
{
	if (UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		TurnManagerSubsystem->EndCurrentUnitTurn();
	}
}

void AHexagonalGamePlayerController::OnCurrentUnitChanged(AHexBattleUnit* NewUnit)
{
	ClearHoveredCell();
	ClearMovementRangePreview();

	if (!IsValid(NewUnit))
	{
		return;
	}

	ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetPawn());
	if (IsValid(CameraPawn))
	{
		CameraPawn->SetFocusTarget(NewUnit);
	}

	ShowMovementRangeForUnit(NewUnit);

	if (!NewUnit->IsPlayerControllable())
	{
		// 暂时还没做敌人AI，所以直接跳过敌人回合，后续会改成真正的AI逻辑。
		GetWorld()->GetTimerManager().SetTimer(
			EnemyTurnDelayForTestTimerHandle,
			this,
			&AHexagonalGamePlayerController::SimulateEnemyForTest,
			1.5f,
			false
		);	
		return;
	}
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, "PlayerController: It's my turn!");
}

void AHexagonalGamePlayerController::HandleEdgeScroll(float DeltaTime)
{
	if (!bAllowEdgeScroll)
		return;
	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return;
	}
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return;
	}
	
	auto CalcEdgeScrollStrength = [this](float DistanceToEdge)
	{
		if (DistanceToEdge > EdgeScrollBeginThreshold)
		{
			return 0.f;
		}

		const float Alpha =
			(EdgeScrollBeginThreshold - DistanceToEdge) /
			(EdgeScrollBeginThreshold - EdgeScrollMaxSpeedThreshold);

		return FMath::Clamp(Alpha, 0.f, 1.f);
	};
	float LeftStrength = CalcEdgeScrollStrength(MouseX);
	float RightStrength = CalcEdgeScrollStrength(ViewportSizeX - MouseX);
	float TopStrength = CalcEdgeScrollStrength(MouseY);
	float BottomStrength = CalcEdgeScrollStrength(ViewportSizeY - MouseY);
	const FVector2D ScreenInput(
	  RightStrength - LeftStrength,
	  TopStrength - BottomStrength
  );
	const float ScrollStrength = FMath::Max(
	FMath::Max(LeftStrength, RightStrength),
	FMath::Max(TopStrength, BottomStrength)
	);
	if (ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetPawn()))
	{
		CameraPawn->MoveByEdgeScrollInput(ScreenInput, ScrollStrength, DeltaTime);
	}
}

void AHexagonalGamePlayerController::TestCommandFunction()
{
	// 暂时测试结束当前单位回合的功能，后续会改成真正的测试命令。
	if (UTurnManagerSubsystem* TurnManagerSubsystem = GetWorld()->GetSubsystem<UTurnManagerSubsystem>())
	{
		TurnManagerSubsystem->EndCurrentUnitTurn();
	}
}

void AHexagonalGamePlayerController::StartCameraOrbit()
{
	bIsCameraOrbiting = true;
	GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Blue, "Camera Orbit Started");
}

void AHexagonalGamePlayerController::EndCameraOrbit()
{
	bIsCameraOrbiting = false;
	GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Blue, "Camera Orbit Ended");
}

void AHexagonalGamePlayerController::HandleCameraZoom(const FInputActionValue& Value)
{
	float v = Value.Get<float>();
	ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetPawn());
	if (CameraPawn)
	{
		CameraPawn->AdjustDesiredArmLength(v);
	}
}

void AHexagonalGamePlayerController::HandleCameraOrbit(const FInputActionValue& Value)
{
	if (bIsCameraOrbiting)
	{
		float v = Value.Get<float>();
		ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetPawn());
		if (CameraPawn)
		{
			CameraPawn->AddOrbitYawInput(v);
		}
	}
}

void AHexagonalGamePlayerController::UpdateHoveredCell()
{
	UHexGridSubsystem* GridSubsystem = GetWorld()->GetSubsystem<UHexGridSubsystem>();
	if (!GridSubsystem)
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		ClearHoveredCell();
		return;
	}

	FHexCoord NewHoveredCell;
	if (!GridSubsystem->GetCellAtWorldLocation(HitResult.Location, NewHoveredCell))
	{
		ClearHoveredCell();
		return;
	}

	if (bHoveredCell && CurrentHoveredCell == NewHoveredCell)
	{
		return;
	}

	ClearHoveredCell();

	CurrentHoveredCell = NewHoveredCell;
	bHoveredCell = true;
	RestoreCellVisual(CurrentHoveredCell);
}

void AHexagonalGamePlayerController::ShowMovementRangeForUnit(AHexBattleUnit* NewUnit)
{
	if (!IsValid(NewUnit))
	{
		return;
	}

	UHexGridSubsystem* GridSubsystem = GetWorld()->GetSubsystem<UHexGridSubsystem>();
	if (!GridSubsystem)
	{
		return;
	}

	const int32 MoveRange = NewUnit->GetMoveRange();
	if (MoveRange <= 0)
	{
		return;
	}

	CurrentMovementRangeCells = GridSubsystem->GetReachableCells(NewUnit->GetCurrentHexCoord(), MoveRange);
	for (const FHexCoord& Coord : CurrentMovementRangeCells)
	{
		GridSubsystem->SetCellDecalColor(Coord, MovementRangeColor);
		GridSubsystem->SetDecalVisible(Coord, true);
	}
}

void AHexagonalGamePlayerController::ClearHoveredCell()
{
	if (!bHoveredCell)
	{
		return;
	}

	const FHexCoord OldHoveredCell = CurrentHoveredCell;
	bHoveredCell = false;
	RestoreCellVisual(OldHoveredCell);
}

void AHexagonalGamePlayerController::ClearMovementRangePreview()
{
	ClearPathPreview();

	UHexGridSubsystem* GridSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UHexGridSubsystem>() : nullptr;
	if (!GridSubsystem)
	{
		CurrentMovementRangeCells.Empty();
		return;
	}

	for (const FHexCoord& Coord : CurrentMovementRangeCells)
	{
		GridSubsystem->SetDecalVisible(Coord, false);
		GridSubsystem->ResetCellDecalColor(Coord);
	}

	CurrentMovementRangeCells.Empty();
}

void AHexagonalGamePlayerController::ClearPathPreview()
{
	if (CurrentPathPreview.IsEmpty())
	{
		return;
	}

	const TArray<FHexCoord> OldPathPreview = CurrentPathPreview;
	CurrentPathPreview.Empty();

	for (const FHexCoord& Coord : OldPathPreview)
	{
		RestoreCellVisual(Coord);
	}
}

void AHexagonalGamePlayerController::RestoreCellVisual(const FHexCoord& Coord)
{
	UHexGridSubsystem* GridSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UHexGridSubsystem>() : nullptr;
	if (!GridSubsystem)
	{
		return;
	}

	if (bHoveredCell && CurrentHoveredCell == Coord)
	{
		GridSubsystem->SetCellDecalColor(Coord, FLinearColor::Yellow);
		GridSubsystem->SetDecalVisible(Coord, true);
		return;
	}

	if (CurrentPathPreview.Contains(Coord))
	{
		GridSubsystem->SetCellDecalColor(Coord, PathPreviewColor);
		GridSubsystem->SetDecalVisible(Coord, true);
		return;
	}

	if (CurrentMovementRangeCells.Contains(Coord))
	{
		GridSubsystem->SetCellDecalColor(Coord, MovementRangeColor);
		GridSubsystem->SetDecalVisible(Coord, true);
		return;
	}

	GridSubsystem->SetDecalVisible(Coord, false);
	GridSubsystem->ResetCellDecalColor(Coord);
}
