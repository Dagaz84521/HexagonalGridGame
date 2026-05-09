#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/HexCoord.h" 
#include "HexMathLibrary.generated.h"

// 定义六个方向的枚举，方便获取邻居
UENUM(BlueprintType)
enum class EHexDirection : uint8
{
	Right,
	TopRight,
	TopLeft,
	Left,
	BottomLeft,
	BottomRight
};

UCLASS()
class HEXGRIDCORE_API UHexMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FHexCoord Add(const FHexCoord& A, const FHexCoord& B);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FHexCoord GetDirectionOffset(EHexDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FHexCoord GetNeighbour(const FHexCoord& Hex, EHexDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static TArray<FHexCoord> GetAllNeighbours(const FHexCoord& Hex);
	
	// --- 距离寻路 ---
	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static int32 GetDistance(const FHexCoord& A, const FHexCoord& B);

	// --- 空间转换 ---
	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FVector2D HexToLocalSpace(const FHexCoord& Hex, float HexSize);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FHexCoord WorldToHex(const FVector2D& Point, float HexSize);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static EHexDirection GetFacingDirection(const FHexCoord& From, const FHexCoord& To);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static float GetDirectionYaw(EHexDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Hex|Math")
	static FRotator GetDirectionRotation(EHexDirection Direction, float YawOffset = 0.0f);
	
	
private:
	static const FHexCoord DirectionOffsets[6];
};
