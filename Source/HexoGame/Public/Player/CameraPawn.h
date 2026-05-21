// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class HEXOGAME_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* Camera;

	UFUNCTION()
	void MoveByEdgeScrollInput(const FVector2D& ScreenInput, float ScrollStrength, float DeltaTime);

	UFUNCTION()
	void SetFocusTarget(AActor* NewFocusTarget);

	UPROPERTY(EditAnywhere, Category="Camera|Follow")
	float FollowInterpSpeed = 5.0f;
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AActor> FocusTarget = nullptr;

	void UpdateFocusFollow(float DeltaTime);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess = "true"))
	float MaxScrollSpeed = 1000.f;
	// 当鼠标距离边缘的距离在 EdgeScrollBeginThreshold 和 EdgeScrollMaxSpeedThreshold 之间时，滚动速度将根据这个值线性插值。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess = "true"))
	float MinScrollSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "Camera|Bounds", meta=(AllowPrivateAccess = "true"))
	FVector2D MinCameraBounds = FVector2D(-5000.0f, -5000.0f);
	UPROPERTY(EditAnywhere, Category = "Camera|Bounds", meta=(AllowPrivateAccess = "true"))
	FVector2D MaxCameraBounds = FVector2D(5000.0f, 5000.0f);
	
};
