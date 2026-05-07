#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexCoord.h"
#include "HexGridDataAsset.h"
#include "HexGridMap.generated.h"

class AHexUnitBase;
class UDecalComponent;
class UMaterialInterface;
class USceneComponent;
class UHexPathFinderComponent;

UCLASS()
class HEXGRIDCORE_API AHexGridMap : public AActor
{
	GENERATED_BODY()
	
public:
	AHexGridMap();

	virtual void BeginPlay() override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Hex|DataAsset")
	void LoadFromDataAsset();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Hex|DataAsset")
	void BuildGridVisuals();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Hex|DataAsset")
	void ClearGrid();

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

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool CanMoveBetweenCells(const FHexCoord& FromCoord, const FHexCoord& ToCoord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	AHexUnitBase* GetUnitAtCoord(const FHexCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool PlaceUnitAtCoord(AHexUnitBase* Unit, const FHexCoord& Coord); 

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool MoveUnitToCoord(AHexUnitBase* Unit, const FHexCoord& TargetCoord);

	UFUNCTION(BlueprintCallable, Category = "Hex|Cell")
	bool MoveUnitToCoordNew(AHexUnitBase* Unit, const FHexCoord& TargetCoord);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset")
	UHexGridDataAsset* GridDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset")
	float HexSize = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset")
	float GridYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|DataAsset")
	TMap<FHexCoord, FHexCellData> GridData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	FVector DecalSize = FVector(100.0f, 100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	float DecalYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Pathfinding", meta = (ClampMin = "0.0"))
	float MaxStepHeight = 100.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	FLinearColor DefaultDecalColor = FLinearColor::Black;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex|Visual")
	TMap<FHexCoord, AHexUnitBase*> HexUnits;
private:
	UPROPERTY()
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hex|Pathfinding")
	UHexPathFinderComponent* PathFinderComponent;

	UPROPERTY()
	TArray<UDecalComponent*> SpawnedDecals;

	UPROPERTY()
	TMap<FHexCoord, UDecalComponent*> DecalsMap;
};
