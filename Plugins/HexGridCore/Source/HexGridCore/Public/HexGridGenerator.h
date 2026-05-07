#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "HexCoord.h"
#include "HexGridDataAsset.h"
#include "HexGridGenerator.generated.h"

class UBoxComponent;
class UDecalComponent;
class UMaterialInterface;

UCLASS()
class HEXGRIDCORE_API AHexGridGenerator : public AActor
{
	GENERATED_BODY()

public:
	AHexGridGenerator();
	UFUNCTION()
	virtual void BeginPlay() override;
	
	// 编辑器操作
	UFUNCTION(CallInEditor, Category = "Hex")
	void PreviewGridWires();

	UFUNCTION(CallInEditor, Category = "Hex")
	void GenerateRuntimeGrid();

	UFUNCTION(CallInEditor, Category = "Grid Generation")
	void GenerateGrid();

	UFUNCTION(CallInEditor, Category = "Hex")
	void ClearGrid();

	// 颜色设置
	UFUNCTION(BlueprintCallable, Category = "Hex")
	bool SetCellDecalColor(const FHexCoord& Coord, const FLinearColor& Color);
	UFUNCTION(BlueprintCallable, Category = "Hex")
	bool SetCellDecalEnableCenterCircle(const FHexCoord& Coord, bool bEnable);
	// 重置颜色
	UFUNCTION(BlueprintCallable, Category = "Hex")
	bool ResetCellDecalColor(const FHexCoord& Coord) { return SetCellDecalColor(Coord, DefaultDecalColor); }
	UFUNCTION(BlueprintCallable, Category = "Hex")
	bool GetCellAtWorldLocation(const FVector& WorldLocation, FHexCoord& OutCoord) const;

	// 组件
	// 划定生成范围的包围盒。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex")
	UBoxComponent* BoundsComponent;

	// 网格设置
	// 六边形大小，表示中心到顶点的距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex")
	float HexSize = 100.0f;

	// 当前区域内生成的有效格子。
	UPROPERTY(VisibleAnywhere, Category = "Hex")
	TMap<FHexCoord, FHexCellData> GridData;

	// 地面检测设置
	// 射线检测起始高度，相对于格子中心上下偏移。
	UPROPERTY(EditAnywhere, Category = "Hex|Trace")
	float TraceHeight = 1000.0f;

	// 最大允许通行坡度，超过该角度视为不可通行。
	UPROPERTY(EditAnywhere, Category = "Hex|Trace")
	float MaxWalkableSlopeAngle = 45.0f;

	// 用于检测地形的碰撞通道。
	UPROPERTY(EditAnywhere, Category = "Hex|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// 顶点与中心点高度差在此范围内视为相似。
	UPROPERTY(EditAnywhere, Category = "Hex|Trace")
	float HeightTolerance = 20.0f;

	// 至少需要多少个六边形顶点与中心点高度相似。
	UPROPERTY(EditAnywhere, Category = "Hex|Trace")
	int32 MinRequiredSimilarCorners = 3;

	// 碰撞设置
	UPROPERTY(EditAnywhere, Category = "Hex|Collision")
	float AgentRadius = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Hex|Collision")
	float AgentHalfHeight = 90.0f;

	// 稍微抬高生成位置，减少贴近地面导致的误判或摩擦。
	UPROPERTY(EditAnywhere, Category = "Hex|Collision")
	float GroundOffset = 5.0f;

	// 可视化设置
	UPROPERTY(EditAnywhere, Category = "Hex|Visual")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditAnywhere, Category = "Hex|Visual")
	FVector DecalSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, Category = "Hex|Visual")
	float DecalYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Grid Generation")
	UHexGridDataAsset* TargetGridDataAsset;
private:
	UPROPERTY()
	TArray<UDecalComponent*> SpawnedDecals;

	UPROPERTY()
	TMap<FHexCoord, UDecalComponent*> DecalsMap; // 方便后续管理贴花

	UPROPERTY()
	FLinearColor DefaultDecalColor = FLinearColor::Black;
};
