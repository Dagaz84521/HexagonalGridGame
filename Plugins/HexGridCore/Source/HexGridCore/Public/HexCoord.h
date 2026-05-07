#pragma once
#include "CoreMinimal.h"
#include "HexCoord.generated.h"

USTRUCT(BlueprintType)
struct FHexCoord
{
	GENERATED_BODY()

	// 轴向坐标的两个核心维度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex")
	int32 Q;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex")
	int32 R;

	// 构造函数
	FHexCoord() : Q(0), R(0) {}
	FHexCoord(int32 InQ, int32 InR) : Q(InQ), R(InR) {}

	// 获取隐式的第三个坐标轴 S
	int32 GetS() const { return -Q - R; }

	// 重载判等运算符，方便放入 TSet 或 TMap 作为哈希键值
	bool operator==(const FHexCoord& Other) const
	{
		return Q == Other.Q && R == Other.R;
	}
};

USTRUCT(BlueprintType)
struct FHexCellData
{
	GENERATED_BODY()

	// 逻辑坐标
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex")
	FHexCoord Coordinate;

	// 真实世界的三维坐标（中心点）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex")
	FVector WorldLocation;

	// 是否可通行
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex")
	bool bIsPassable;

	// 可以在这里扩展更多属性：比如地形类型（草地、泥地）、移动消耗等

	FHexCellData() : WorldLocation(FVector::ZeroVector), bIsPassable(true) {}
};

// 为 FHexCoord 实现全局哈希函数，这是在 UE 中使用 TMap/TSet 必须的一步
FORCEINLINE uint32 GetTypeHash(const FHexCoord& Hex)
{
	return HashCombine(GetTypeHash(Hex.Q), GetTypeHash(Hex.R));
}