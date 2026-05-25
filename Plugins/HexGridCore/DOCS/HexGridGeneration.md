# 六边形网格的生成

## 目标

对于一个给定的场景关卡，能够支持快速地生成六边形网格配置。

有如下几点要求：

- 能够支持划定区域，只在规定的区域内生成网格。
- 在无法站下一个棋子的位置应该可以将网格设置成不可通行。
- 应该记录高度，用于后续寻路使用。
- 能够支持将生成结果保存为可复用的数据资产，而不是每次运行时重新扫描场景。

## 实现方式

当前六边形网格的生成主要由 `AHexGridGenerator` 负责。

它是一个放置在关卡中的编辑器辅助 Actor，通过自身的 `BoundsComponent` 划定生成范围，并根据配置好的六边形尺寸、地形检测参数和目标 DataAsset 生成网格数据。

### 1. 使用 BoundsComponent 划定生成区域

`AHexGridGenerator` 内部有一个 `UBoxComponent`：

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hex")
UBoxComponent* BoundsComponent;
```

生成网格时，会读取这个 Box 的范围和变换：

```cpp
const FVector Extent = BoundsComponent->GetScaledBoxExtent();
const FTransform BoundsTransform = BoundsComponent->GetComponentTransform();
```

后续所有候选格子都会先在 Bounds 的局部空间中计算。

这样做的好处是：

- 可以直接在关卡中拖动、缩放、旋转生成区域。
- 只生成 Box 范围内的格子。
- 即使生成区域旋转，网格也能跟随该区域的朝向。

### 2. 遍历六边形轴向坐标

当前项目使用轴向坐标表示六边形格子：

```cpp
FHexCoord(q, r)
```

生成时会先根据 Bounds 的大小和 `HexSize` 估算一个足够覆盖区域的最大搜索半径：

```cpp
const int32 MaxRadius = FMath::CeilToInt((Extent.X + Extent.Y) / HexSize);
```

然后遍历 `q` 和 `r`：

```cpp
for (int32 q = -MaxRadius; q <= MaxRadius; ++q)
{
    const int32 r1 = FMath::Max(-MaxRadius, -q - MaxRadius);
    const int32 r2 = FMath::Min(MaxRadius, -q + MaxRadius);

    for (int32 r = r1; r <= r2; ++r)
    {
        const FHexCoord CurrentCoord(q, r);
    }
}
```

这样可以得到一批候选六边形格子。

### 3. 将六边形坐标转换为局部位置

每个候选格子会通过 `UHexMathLibrary::HexToLocalSpace()` 转换为局部 2D 坐标：

```cpp
const FVector2D LocalPos2D = UHexMathLibrary::HexToLocalSpace(CurrentCoord, HexSize);
```

然后根据 Bounds 的局部范围做裁剪：

```cpp
if (FMath::Abs(LocalPos2D.X) > Extent.X || FMath::Abs(LocalPos2D.Y) > Extent.Y)
{
    continue;
}
```

这一步保证最终只保留生成区域内的格子。

### 4. 将局部位置转换为世界位置

通过 Bounds 的 Transform，将局部中心点转换到世界坐标：

```cpp
const FVector LocalCenter(LocalPos2D.X, LocalPos2D.Y, 0.0f);
const FVector WorldCenterBase = BoundsTransform.TransformPositionNoScale(LocalCenter);
```

这里使用 `TransformPositionNoScale()`，是为了让生成区域的缩放只影响生成范围，而不会把六边形本身拉伸变形。

### 5. 通过射线检测获取地面高度

对于每个候选格子，会从中心点上方向下做射线检测：

```cpp
FHitResult CenterHit;
const bool bHitCenter = GetWorld()->LineTraceSingleByChannel(
    CenterHit,
    WorldCenterBase + FVector(0.0f, 0.0f, TraceHeight),
    WorldCenterBase - FVector(0.0f, 0.0f, TraceHeight),
    TraceChannel,
    QueryParams
);
```

如果中心点命中地面，就使用命中位置作为该格子的基础高度。

此外，还会检测六边形的 6 个顶点，用来判断这个格子的地形是否足够平整。

### 6. 判断格子是否可通行

当前可通行判断主要包含三部分。

第一，中心点必须命中地面。

如果中心点没有命中，说明该位置没有有效地面，格子不可通行。

第二，六边形顶点高度需要和中心点足够接近。

生成器会采样 6 个顶点，如果顶点和中心点的高度差小于 `HeightTolerance`，就认为该点与中心点高度相似。

```cpp
const float HeightDiff = FMath::Abs(CornerHit.Location.Z - CenterHit.Location.Z);
if (HeightDiff <= HeightTolerance)
{
    ++SimilarPointsCount;
    HeightSum += CornerHit.Location.Z;
}
```

如果相似点数量不足，则格子不可通行：

```cpp
if (!bHitCenter || SimilarPointsCount < (MinRequiredSimilarCorners + 1))
{
    bTerrainPassable = false;
}
```

第三，地面坡度不能超过限制。

通过中心点命中的法线计算坡度：

```cpp
const float SlopeAngle = FMath::RadiansToDegrees(
    FMath::Acos(FVector::DotProduct(CenterHit.ImpactNormal, FVector::UpVector))
);
```

如果坡度大于 `MaxWalkableSlopeAngle`，则格子不可通行。

### 7. 检测棋子是否能站下

在地形本身可通行之后，还会检查该位置是否有足够空间放置棋子。

当前实现使用球形 Overlap：

```cpp
const FVector CheckLocation = GroundLocation + FVector(0.0f, 0.0f, UnitPlacementCheckHeight);
const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(UnitPlacementCheckRadius);
```

然后调用：

```cpp
World->OverlapAnyTestByChannel(
    CheckLocation,
    FQuat::Identity,
    UnitPlacementTraceChannel,
    CollisionShape,
    QueryParams
);
```

如果检测到阻挡，就说明该格子虽然有地面，但棋子站不下，因此会被标记为不可通行。

### 8. 生成格子数据

每个有效候选格都会生成一份 `FHexCellData`：

```cpp
FHexCellData CellData;
CellData.Coordinate = CurrentCoord;
CellData.WorldLocation = ...;
CellData.bIsPassable = bTerrainPassable && !bPlacementBlocked;
```

其中：

- `Coordinate` 记录六边形逻辑坐标。
- `WorldLocation` 记录格子中心在场景中的位置和高度。
- `bIsPassable` 记录该格子是否可通行。

即使格子不可通行，也会被保存下来，只是 `bIsPassable` 为 `false`。

### 9. 保存到 DataAsset

编辑器生成入口是 `GenerateGrid()`。

生成完成后，会将结果写入目标 `UHexGridDataAsset`：

```cpp
TargetGridDataAsset->Modify();
TargetGridDataAsset->GridData.Empty();
TargetGridDataAsset->HexSize = HexSize;
TargetGridDataAsset->GridYaw = BoundsTransform.GetRotation().Rotator().Yaw;
TargetGridDataAsset->DecalYawOffset = DecalYawOffset;
```

然后逐个写入格子数据：

```cpp
for (const TPair<FHexCoord, FHexCellData>& CellPair : GeneratedGridData)
{
    const FHexCoord& Coord = CellPair.Key;
    TargetGridDataAsset->GridData.Add(FVector2D(Coord.Q, Coord.R), CellPair.Value);
}
```

最后标记资源已修改：

```cpp
TargetGridDataAsset->MarkPackageDirty();
```

这样生成结果就会被保存成可复用的数据资产。运行时只需要读取这个 DataAsset，不需要每次重新扫描场景。

