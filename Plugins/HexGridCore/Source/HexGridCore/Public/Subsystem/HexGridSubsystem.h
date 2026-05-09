// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/HexCoord.h"
#include "HexGridSubsystem.generated.h"

class AHexUnitBase;
class AHexGridMap;
class UDecalComponent;
class UHexGridDataAsset;
class UMaterialInterface;

UCLASS()
class HEXGRIDCORE_API UHexGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	// 从数据资产初始化 Subsystem 内部地图数据。当前用于新路径，不影响已有 AHexGridMap。
	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	void LoadFromDataAsset(UHexGridDataAsset* InGridDataAsset);

	// 迁移过渡入口：把现有 AHexGridMap 的数据复制进 Subsystem，方便逐步切换调用方。
	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	void ImportFromGridMap(AHexGridMap* GridMap);

	// 根据 Subsystem 内部 GridData 生成独立贴花表现。
	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	void BuildGridVisuals();

	// 清理 Subsystem 管理的地图、单位占用和贴花，不会清理旧 AHexGridMap。
	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	void ClearGrid();

	// 用最近中心点匹配鼠标/世界位置对应的格子，逻辑保持和 AHexGridMap 一致。
	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	bool GetCellAtWorldLocation(const FVector& WorldLocation, FHexCoord& OutCoord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	bool GetCellData(const FHexCoord& Coord, FHexCellData& OutCellData) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	bool SetCellDecalColor(const FHexCoord& Coord, const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	bool SetCellDecalEnableCenterCircle(const FHexCoord& Coord, bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Hex|DataAsset")
	bool ResetCellDecalColor(const FHexCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool IsValidCell(const FHexCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool IsCellOccupied(const FHexCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool CanCellMoveTo(const FHexCoord& Coord) const;

	// 判断两格之间是否允许跨越，包含目标格可通行/未占用和高度差限制。
	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool CanMoveBetweenCells(const FHexCoord& FromCoord, const FHexCoord& ToCoord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	AHexUnitBase* GetUnitAtCoord(const FHexCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool PlaceUnitAtCoord(AHexUnitBase* Unit, const FHexCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool MoveUnitToCoord(AHexUnitBase* Unit, const FHexCoord& TargetCoord);

	// 使用 Subsystem 自己的 FindPath 结果驱动单位逐格移动。
	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool MoveUnitToCoordWithPath(AHexUnitBase* Unit, const FHexCoord& TargetCoord);

	// A* 寻路入口，迁移自 UHexPathFinderComponent 的核心逻辑。
	UFUNCTION(BlueprintCallable, Category = "Hex|Pathfinding")
	TArray<FHexCoord> FindPath(const FHexCoord& Start, const FHexCoord& Goal) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset")
	TObjectPtr<UHexGridDataAsset> GridDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	FORCEINLINE void SetDecalMaterial(UMaterialInterface* InMaterial) { DecalMaterial = InMaterial; }

private:
	// 地图基础参数和格子数据目前是 Subsystem 的独立副本，避免影响旧 Map 的运行状态。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset", meta = (AllowPrivateAccess = "true"))
	float HexSize = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset", meta = (AllowPrivateAccess = "true"))
	float GridYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset", meta = (AllowPrivateAccess = "true"))
	TMap<FHexCoord, FHexCellData> GridData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual", meta = (AllowPrivateAccess = "true"))
	FVector DecalSize = FVector(300.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual", meta = (AllowPrivateAccess = "true"))
	float DecalYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Pathfinding", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float MaxStepHeight = 100.0f;

	// Subsystem 自己生成和管理的贴花缓存。
	UPROPERTY()
	TArray<UDecalComponent*> SpawnedDecals;

	UPROPERTY()
	TMap<FHexCoord, UDecalComponent*> DecalsMap;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	FLinearColor DefaultDecalColor = FLinearColor::Black;

	// 单位占用表迁移自 AHexGridMap，后续玩家输入可以直接查询 Subsystem。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	TMap<FHexCoord, AHexUnitBase*> HexUnits;
};
