// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/HexagonalGamePlayerController.h"
#include "Game/HexagonalGameMode.h"
#include "Pawn/HexBattleUnit.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Player/CameraPawn.h"
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
	}
}

void AHexagonalGamePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleEdgeScroll(DeltaTime);
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
	if (!IsValid(NewUnit))
		return;
	ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetPawn());
	if (!IsValid(CameraPawn))
		return;
	CameraPawn->SetFocusTarget(NewUnit);
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
