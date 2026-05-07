// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexCoord.h"
#include "HexMathLibrary.h"
#include "HexUnitTypes.h"
#include "GameFramework/Actor.h"
#include "HexUnitBase.generated.h"

class USceneComponent;

UCLASS()
class HEXGRIDCORE_API AHexUnitBase : public APawn
{
	GENERATED_BODY()
	
public:	
	AHexUnitBase();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit|Movement")
	void MoveAlongWorldPath(const TArray<FVector>& InPathPoints);

	void MoveAlongHexPath(const FHexCoord& StartCoord, const TArray<FHexCoord>& InPathCoords, const TArray<FVector>& InPathPoints);

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit|Movement")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	FHexCoord GetCurrentHexCoord() const { return CurrentHexCoord; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	void SetCurrentHexCoord(const FHexCoord& InCoord) { CurrentHexCoord = InCoord; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	EHexUnitFaction GetFaction() const { return Faction; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	bool IsPlayerControllable() const { return Faction == EHexUnitFaction::PlayerControllable; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	EHexDirection GetFacingDirection() const { return FacingDirection; }

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	void SetFacingDirection(EHexDirection NewDirection);

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	void ApplyFacingRotation();

	UFUNCTION(BlueprintCallable, Category = "Hex|Unit")
	void SetFaction(const EHexUnitFaction& newFaction) {Faction = newFaction;}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FHexCoord CurrentHexCoord;
	UPROPERTY(EditAnywhere)
	EHexUnitFaction Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Unit|Movement")
	float MoveSpeed = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|Unit|Movement")
	bool bIsMoving = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Unit|Facing")
	EHexDirection FacingDirection = EHexDirection::Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Unit|Facing")
	float FacingYawOffset = -90.0f; // 默认偏移，使得模型面向正确方向

	UPROPERTY()
	TArray<FVector> MovePathPoints;

	UPROPERTY()
	TArray<FHexCoord> MovePathCoords;

	FHexCoord MovePathStartCoord;
	int32 MovePathIndex = 0;

	void UpdateFacingForMoveSegment();
public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* Mesh;
};
