// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HexagonalGamePlayerController.generated.h"

class UInputAction;
class ACameraPawn;
class AHexBattleUnit;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class HEXOGAME_API AHexagonalGamePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	void SimulateEnemyForTest();

	UFUNCTION()
	void OnCurrentUnitChanged(AHexBattleUnit* NewUnit);

	UFUNCTION()
	void HandleEdgeScroll(float DeltaTime);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* TestAction;

	UFUNCTION()
	void TestCommandFunction();

private:
	// 在边缘滚动时，鼠标距离边缘的距离达到这个值时，滚动速度将达到最大。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess = "true"))
	float EdgeScrollMaxSpeedThreshold = 30.f;
	// 在边缘滚动时，鼠标距离边缘的距离达到这个值时，开始滚动。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess = "true"))
	float EdgeScrollBeginThreshold = 70.f;
	// 当鼠标距离边缘的距离小于 EdgeScrollMaxSpeedThreshold 时，滚动速度将达到这个值。
	// 是否允许边缘滚动。
	bool bAllowEdgeScroll = false;

	FTimerHandle EnemyTurnDelayForTestTimerHandle;
};
