// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlayerController.h"
#include "HexGridMap.h"
#include "HexGridGenerator.h"
#include "HexPathFinderComponent.h"
#include "HexUnitBase.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"


void AHexPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 缓存棋盘和寻路组件引用，后续输入逻辑都围绕 Map 工作。
	GridGenerator = Cast<AHexGridGenerator>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AHexGridGenerator::StaticClass())
	);
	Map = Cast<AHexGridMap>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AHexGridMap::StaticClass())
	);
	PathFinder = Map ? Map->FindComponentByClass<UHexPathFinderComponent>() : nullptr;
	bShowMouseCursor = true;
	EnterIdleState();
}

void AHexPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 每帧只处理鼠标当前指向的格子，用于悬停高亮和路径预览。
	if (!Map)
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		return;
	}

	FHexCoord NewHoveredCoord;
	if (!Map->GetCellAtWorldLocation(HitResult.Location, NewHoveredCoord))
	{
		return;
	}

	if (bHasHoveredCoord && CurrentHoveredCoord == NewHoveredCoord)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Q:%d, R:%d"), NewHoveredCoord.Q, NewHoveredCoord.R));

	// 鼠标移到新格子时，先清掉旧格子的悬停表现。
	if (bHasHoveredCoord)
	{
		Map->SetCellDecalEnableCenterCircle(CurrentHoveredCoord, false);
		Map->ResetCellDecalColor(CurrentHoveredCoord);
	}

	// 选中单位后，鼠标悬停到哪里就预览到哪里的路径。
	if (InputState == EHexInputState::UnitSelected)
	{
		ShowPreviewPath(NewHoveredCoord);
	}
	else if (CurrentPath.Num() > 0)
	{
		ClearPreviewPath();
	}

	// 最后再设置当前格子的悬停表现，避免被路径清理逻辑覆盖。
	Map->SetCellDecalColor(NewHoveredCoord, FLinearColor::Green);
	Map->SetCellDecalEnableCenterCircle(NewHoveredCoord, true);
	CurrentHoveredCoord = NewHoveredCoord;
	bHasHoveredCoord = true;
}

void AHexPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &AHexPlayerController::HandleLeftClick);
	}
}

void AHexPlayerController::HandleLeftClick()
{
	if (!Map)
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		return;
	}

	// 没有点到有效格子时，取消当前选择。
	FHexCoord ClickedCoord;
	if (!Map->GetCellAtWorldLocation(HitResult.Location, ClickedCoord))
	{
		if (InputState == EHexInputState::UnitSelected)
		{
			EnterIdleState();
		}
		return;
	}

	// 点击行为由当前输入状态决定：空闲时尝试选单位，选中单位时尝试切换/移动。
	switch (InputState)
	{
	case EHexInputState::Idle:
		HandleIdleClick(ClickedCoord);
		break;
	case EHexInputState::UnitSelected:
		HandleUnitSelectedClick(ClickedCoord);
		break;
	default:
		break;
	}
}

void AHexPlayerController::HandleIdleClick(const FHexCoord& ClickedCoord)
{
	// 空闲状态下，只允许选择玩家可控制的单位。
	AHexUnitBase* ClickedUnit = Map ? Map->GetUnitAtCoord(ClickedCoord) : nullptr;
	if (!IsValid(ClickedUnit) || !ClickedUnit->IsPlayerControllable())
	{
		return;
	}

	EnterUnitSelectedState(ClickedUnit);
}

void AHexPlayerController::HandleUnitSelectedClick(const FHexCoord& ClickedCoord)
{
	if (!Map || !IsValid(SelectedUnit))
	{
		EnterIdleState();
		return;
	}

	// 再点一次当前单位：取消选择。
	AHexUnitBase* ClickedUnit = Map->GetUnitAtCoord(ClickedCoord);
	if (ClickedUnit == SelectedUnit)
	{
		EnterIdleState();
		return;
	}

	// 点到另一个玩家单位：切换选择。
	if (IsValid(ClickedUnit) && ClickedUnit->IsPlayerControllable())
	{
		EnterUnitSelectedState(ClickedUnit);
		return;
	}

	// 点到空格/目标格：如果存在有效路径，就把单位移动到目标格。
	// 接下来的修改：改成按照PathFinder的结果来逐格移动，之前直接调用Map的MoveUnitToCoord会绕过PathFinder的逻辑，导致一些特殊格子（如河流）无法正确处理。
	const FHexCoord StartCoord = SelectedUnit->GetCurrentHexCoord();
	const bool bHasValidPath = PathFinder && StartCoord != ClickedCoord && PathFinder->FindPath(StartCoord, ClickedCoord).Num() > 0;
	if (bHasValidPath && Map->MoveUnitToCoordNew(SelectedUnit, ClickedCoord))
	{
		EnterIdleState();
	}
}

void AHexPlayerController::EnterIdleState()
{
	// 回到空闲状态时，清理选择和路径预览。
	ClearPreviewPath();
	SelectedUnit = nullptr;
	InputState = EHexInputState::Idle;
}

void AHexPlayerController::EnterUnitSelectedState(AHexUnitBase* Unit)
{
	if (!IsValid(Unit))
	{
		EnterIdleState();
		return;
	}

	SelectedUnit = Unit;
	InputState = EHexInputState::UnitSelected;

	// 如果鼠标已经停在某个格子上，选中单位后立即显示到该格子的路径。
	if (bHasHoveredCoord)
	{
		ShowPreviewPath(CurrentHoveredCoord);
	}
}

void AHexPlayerController::ClearPreviewPath()
{
	if (!Map)
	{
		CurrentPath.Empty();
		return;
	}

	// 只清理路径格子的表现；当前鼠标悬停格保留给 Tick 统一刷新。
	for (const FHexCoord& Coord : CurrentPath)
	{
		if (bHasHoveredCoord && Coord == CurrentHoveredCoord)
		{
			continue;
		}

		Map->SetCellDecalEnableCenterCircle(Coord, false);
		Map->ResetCellDecalColor(Coord);
	}

	CurrentPath.Empty();
}

void AHexPlayerController::ShowPreviewPath(const FHexCoord& GoalCoord)
{
	ClearPreviewPath();

	if (!Map || !PathFinder || !IsValid(SelectedUnit))
	{
		return;
	}

	const FHexCoord StartCoord = SelectedUnit->GetCurrentHexCoord();
	if (StartCoord == GoalCoord)
	{
		return;
	}

	// 预览路径不高亮目标格，目标格通常会被鼠标悬停高亮覆盖成绿色。
	CurrentPath = PathFinder->FindPath(StartCoord, GoalCoord);
	for (int32 Index = 0; Index < CurrentPath.Num(); ++Index)
	{
		const bool bIsGoalCell = Index == CurrentPath.Num() - 1;
		if (bIsGoalCell)
		{
			continue;
		}

		Map->SetCellDecalColor(CurrentPath[Index], FLinearColor::Yellow);
	}
}
