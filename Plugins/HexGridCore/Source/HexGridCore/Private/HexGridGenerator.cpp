#include "HexGridGenerator.h"
#include "HexMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AHexGridGenerator::AHexGridGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // 初始化包围盒，默认给个 1000x1000 的范围
    BoundsComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
    RootComponent = BoundsComponent;
    BoundsComponent->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
    BoundsComponent->SetLineThickness(5.0f);
}

void AHexGridGenerator::BeginPlay()
{
    Super::BeginPlay();
}

void AHexGridGenerator::PreviewGridWires()
{
    GridData.Empty();
    FlushPersistentDebugLines(GetWorld());

    TArray<UTextRenderComponent*> ExistingTexts;
    GetComponents<UTextRenderComponent>(ExistingTexts);
    for (UTextRenderComponent* TextComp : ExistingTexts)
    {
        if (IsValid(TextComp))
        {
            TextComp->DestroyComponent();
        }
    }

    if (!BoundsComponent) return;

    // 获取包围盒的 Extent (半长/半宽)
    FVector Extent = BoundsComponent->GetScaledBoxExtent();
    
    // 【核心修改 1】：获取包含位置、旋转和缩放的完整 Transform
    FTransform ActorTransform = BoundsComponent->GetComponentTransform();

    int32 MaxRadius = FMath::CeilToInt((Extent.X + Extent.Y) / HexSize);

    for (int32 q = -MaxRadius; q <= MaxRadius; ++q)
    {
        int32 r1 = FMath::Max(-MaxRadius, -q - MaxRadius);
        int32 r2 = FMath::Min(MaxRadius, -q + MaxRadius);

        for (int32 r = r1; r <= r2; ++r)
        {
            FHexCoord CurrentCoord(q, r);
            FVector2D LocalPos2D = UHexMathLibrary::HexToLocalSpace(CurrentCoord, HexSize);

            // 在未旋转的局部空间中进行完美裁剪
            if (FMath::Abs(LocalPos2D.X) > Extent.X || FMath::Abs(LocalPos2D.Y) > Extent.Y)
            {
                continue;
            }

            // 构建 3D 局部中心点坐标 (Z=0)
            FVector LocalCenter(LocalPos2D.X, LocalPos2D.Y, 0.0f);

            // 【核心修改 2】：将局部坐标转换到世界坐标，使其跟随 Actor 旋转！
            // 使用 TransformPositionNoScale 确保六边形不会因为你缩放了包围盒而被拉伸变形
            FVector WorldPos = ActorTransform.TransformPositionNoScale(LocalCenter);

            FHexCellData CellData;
            CellData.Coordinate = CurrentCoord;
            CellData.WorldLocation = WorldPos;
            CellData.bIsPassable = true; 

            GridData.Add(CurrentCoord, CellData);

            // 计算顶点并绘制
            FVector Corners[6];
            for (int32 i = 0; i < 6; ++i)
            {
                float AngleDeg = 60.0f * i + 30.0f;
                float AngleRad = FMath::DegreesToRadians(AngleDeg);
                
                // 先算出顶点相对于网格原点的局部坐标
                FVector LocalCornerPos = LocalCenter + FVector(HexSize * FMath::Cos(AngleRad), HexSize * FMath::Sin(AngleRad), 0.0f);
                
                // 【核心修改 3】：将顶点也转换到世界坐标
                Corners[i] = ActorTransform.TransformPositionNoScale(LocalCornerPos);
            }
        
            for (int32 i = 0; i < 6; ++i)
            {
                int32 NextIndex = (i + 1) % 6;
                DrawDebugLine(GetWorld(), Corners[i], Corners[NextIndex], FColor::Green, true, -1.0f, 0, 2.0f);
            }

            DrawDebugPoint(GetWorld(), WorldPos, 5.0f, FColor::Red, true);
            // 【修改为使用 3D 文本组件】
            FString CoordText = FString::Printf(TEXT("%d, %d"), CurrentCoord.Q, CurrentCoord.R);

            UTextRenderComponent* TextRender = NewObject<UTextRenderComponent>(this);
            TextRender->SetText(FText::FromString(CoordText));
            TextRender->SetWorldSize(30.0f); // 控制字的大小
            TextRender->SetTextRenderColor(FColor::White);
            TextRender->SetHorizontalAlignment(EHTA_Center);
            TextRender->SetVerticalAlignment(EVRTA_TextCenter);
            
            TextRender->SetupAttachment(RootComponent);
            
            TextRender->SetAbsolute(false, false, true);
            
            TextRender->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
            
            TextRender->RegisterComponent();
            
            TextRender->SetWorldLocation(WorldPos + FVector(0.0f, 0.0f, 10.0f));
        }
    }
}

void AHexGridGenerator::ClearGrid()
{
    GridData.Empty();
    for (UDecalComponent* Decal : SpawnedDecals)
    {
        if (IsValid(Decal))
        {
            Decal->DestroyComponent();
        }
    }
    SpawnedDecals.Empty();
    DecalsMap.Empty();
}

bool AHexGridGenerator::SetCellDecalColor(const FHexCoord& Coord, const FLinearColor& Color)
{
    UDecalComponent** Decal = DecalsMap.Find(Coord);
    if (!Decal || !IsValid(*Decal))
    {
        return false;
    }

    UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>((*Decal)->GetDecalMaterial());
    if (!DynamicMaterial)
    {
        DynamicMaterial = (*Decal)->CreateDynamicMaterialInstance();
    }

    if (!DynamicMaterial)
    {
        return false;
    }

    DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
    return true;
}

bool AHexGridGenerator::SetCellDecalEnableCenterCircle(const FHexCoord& Coord, bool bEnable)
{
    UDecalComponent** Decal = DecalsMap.Find(Coord);
    if (!Decal || !IsValid(*Decal))
    {
        return false;
    }

    UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>((*Decal)->GetDecalMaterial());
    if (!DynamicMaterial)
    {
        DynamicMaterial = (*Decal)->CreateDynamicMaterialInstance();
    }

    if (!DynamicMaterial)
    {
        return false;
    }

    DynamicMaterial->SetScalarParameterValue(TEXT("EnableCenterCircle"), bEnable ? 1.0f : 0.0f);
    return true;
}

bool AHexGridGenerator::GetCellAtWorldLocation(const FVector& WorldLocation, FHexCoord& OutCoord) const
{
    if (!BoundsComponent || HexSize <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const FTransform BoundsTransform = BoundsComponent->GetComponentTransform();

    // 生成格子时用了 TransformPositionNoScale，这里要用对应的逆变换。
    const FVector LocalLocation = BoundsTransform.InverseTransformPositionNoScale(WorldLocation);

    const FHexCoord Coord = UHexMathLibrary::WorldToHex(
        FVector2D(LocalLocation.X, LocalLocation.Y),
        HexSize
    );

    if (!GridData.Contains(Coord))
    {
        return false;
    }

    OutCoord = Coord;
    return true;
}


void AHexGridGenerator::GenerateRuntimeGrid()
{
    // 1. 清理旧数据和旧贴花
    ClearGrid();

    if (!BoundsComponent) return;

    // 获取包围盒属性及变换矩阵
    FVector Extent = BoundsComponent->GetScaledBoxExtent();
    FTransform ActorTransform = BoundsComponent->GetComponentTransform();
    
    // 计算足以覆盖包围盒的最大搜索半径
    int32 MaxRadius = FMath::CeilToInt((Extent.X + Extent.Y) / HexSize);

    // 遍历轴向坐标系
    for (int32 q = -MaxRadius; q <= MaxRadius; ++q)
    {
        int32 r1 = FMath::Max(-MaxRadius, -q - MaxRadius);
        int32 r2 = FMath::Min(MaxRadius, -q + MaxRadius);

        for (int32 r = r1; r <= r2; ++r)
        {
            FHexCoord CurrentCoord(q, r);
            // 转换到局部 2D 空间
            FVector2D LocalPos2D = UHexMathLibrary::HexToLocalSpace(CurrentCoord, HexSize);

            // 裁剪包围盒外的格子
            if (FMath::Abs(LocalPos2D.X) > Extent.X || FMath::Abs(LocalPos2D.Y) > Extent.Y)
            {
                continue;
            }

            // 计算基准世界坐标（不带高度缩放）
            FVector LocalCenter(LocalPos2D.X, LocalPos2D.Y, 0.0f);
            FVector WorldCenterBase = ActorTransform.TransformPositionNoScale(LocalCenter);

            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            // --- 采样点 0: 中心点 (作为高度基准) ---
            FHitResult CenterHit;
            bool bHitCenter = GetWorld()->LineTraceSingleByChannel(
                CenterHit, 
                WorldCenterBase + FVector(0, 0, TraceHeight), 
                WorldCenterBase - FVector(0, 0, TraceHeight), 
                TraceChannel, QueryParams
            );

            if (!bHitCenter) continue;

            float CenterZ = CenterHit.Location.Z;
            float HeightSum = CenterZ; // 用于后续计算平均高度
            int32 SimilarPointsCount = 1; // 中心点本身算作第 1 个相似点

            // --- 采样点 1-6: 六个顶点投票 ---
            for (int32 i = 0; i < 6; ++i)
            {
                // 计算顶点局部坐标（尖顶六边形：起点 30 度）
                float AngleRad = FMath::DegreesToRadians(60.0f * i + 30.0f);
                FVector CornerLocal = LocalCenter + FVector(HexSize * FMath::Cos(AngleRad), HexSize * FMath::Sin(AngleRad), 0.0f);
                FVector CornerWorldBase = ActorTransform.TransformPositionNoScale(CornerLocal);

                FHitResult CornerHit;
                if (GetWorld()->LineTraceSingleByChannel(
                    CornerHit, 
                    CornerWorldBase + FVector(0, 0, TraceHeight), 
                    CornerWorldBase - FVector(0, 0, TraceHeight), 
                    TraceChannel, QueryParams))
                {
                    // 判断顶点与中心点的高度差
                    float HeightDiff = FMath::Abs(CornerHit.Location.Z - CenterZ);
                    if (HeightDiff <= HeightTolerance)
                    {
                        SimilarPointsCount++;
                        HeightSum += CornerHit.Location.Z;
                    }
                }
            }

            // --- 判定规则 ---
            // 只有当足够多的点（中心点 + 符合要求的顶点）高度一致时，才生成网格
            // SimilarPointsCount 包含中心点，所以阈值应为 MinRequiredSimilarCorners + 1
            if (SimilarPointsCount < (MinRequiredSimilarCorners + 1))
            {
                continue; 
            }

            // 计算该位置的地面坡度
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CenterHit.ImpactNormal, FVector::UpVector)));
            if (SlopeAngle > MaxWalkableSlopeAngle)
            {
                continue;
            }

            // --- 格子数据存储 ---
            FHexCellData CellData;
            CellData.Coordinate = CurrentCoord;
            CellData.bIsPassable = true;
            
            // 使用相似点的平均高度，减少因中心点踩在缝隙里导致的贴花偏移
            float AverageZ = HeightSum / (float)SimilarPointsCount;
            CellData.WorldLocation = FVector(CenterHit.Location.X, CenterHit.Location.Y, AverageZ + GroundOffset);
            
            GridData.Add(CurrentCoord, CellData);

            // --- 贴花生成 ---
            if (DecalMaterial)
            {
                UDecalComponent* DecalComp = NewObject<UDecalComponent>(this);
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterial, this);
                DynamicMaterial->SetVectorParameterValue(TEXT("Color"), DefaultDecalColor);
                DecalComp->SetDecalMaterial(DynamicMaterial);
                DecalComp->DecalSize = DecalSize;
                
                DecalComp->SetupAttachment(RootComponent);
                // 设置绝对缩放，防止受生成器 Actor 缩放影响导致间距错误
                DecalComp->SetAbsolute(false, false, true); 
                DecalComp->RegisterComponent();
                
                DecalComp->SetWorldLocation(CellData.WorldLocation);
                DecalComp->SetWorldRotation(FRotator(-90.0f, ActorTransform.GetRotation().Rotator().Yaw + DecalYawOffset, 0.0f));
                
                SpawnedDecals.Add(DecalComp);
                DecalsMap.Add(CurrentCoord, DecalComp);
            }
        }
    }
}

void AHexGridGenerator::GenerateGrid()
{
    if (!TargetGridDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GenerateGrid failed: TargetGridDataAsset is not set."));
        return;
    }

    if (!BoundsComponent || HexSize <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Warning, TEXT("GenerateGrid failed: BoundsComponent is invalid or HexSize is too small."));
        return;
    }

    TMap<FHexCoord, FHexCellData> GeneratedGridData;

    const FVector Extent = BoundsComponent->GetScaledBoxExtent();
    const FTransform BoundsTransform = BoundsComponent->GetComponentTransform();
    const int32 MaxRadius = FMath::CeilToInt((Extent.X + Extent.Y) / HexSize);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    for (int32 q = -MaxRadius; q <= MaxRadius; ++q)
    {
        const int32 r1 = FMath::Max(-MaxRadius, -q - MaxRadius);
        const int32 r2 = FMath::Min(MaxRadius, -q + MaxRadius);

        for (int32 r = r1; r <= r2; ++r)
        {
            const FHexCoord CurrentCoord(q, r);
            const FVector2D LocalPos2D = UHexMathLibrary::HexToLocalSpace(CurrentCoord, HexSize);

            if (FMath::Abs(LocalPos2D.X) > Extent.X || FMath::Abs(LocalPos2D.Y) > Extent.Y)
            {
                continue;
            }

            const FVector LocalCenter(LocalPos2D.X, LocalPos2D.Y, 0.0f);
            const FVector WorldCenterBase = BoundsTransform.TransformPositionNoScale(LocalCenter);

            FHitResult CenterHit;
            const bool bHitCenter = GetWorld()->LineTraceSingleByChannel(
                CenterHit,
                WorldCenterBase + FVector(0.0f, 0.0f, TraceHeight),
                WorldCenterBase - FVector(0.0f, 0.0f, TraceHeight),
                TraceChannel,
                QueryParams
            );

            if (!bHitCenter)
            {
                continue;
            }

            const float CenterZ = CenterHit.Location.Z;
            float HeightSum = CenterZ;
            int32 SimilarPointsCount = 1;

            for (int32 i = 0; i < 6; ++i)
            {
                const float AngleRad = FMath::DegreesToRadians(60.0f * i + 30.0f);
                const FVector CornerLocal = LocalCenter + FVector(HexSize * FMath::Cos(AngleRad), HexSize * FMath::Sin(AngleRad), 0.0f);
                const FVector CornerWorldBase = BoundsTransform.TransformPositionNoScale(CornerLocal);

                FHitResult CornerHit;
                if (GetWorld()->LineTraceSingleByChannel(
                    CornerHit,
                    CornerWorldBase + FVector(0.0f, 0.0f, TraceHeight),
                    CornerWorldBase - FVector(0.0f, 0.0f, TraceHeight),
                    TraceChannel,
                    QueryParams))
                {
                    const float HeightDiff = FMath::Abs(CornerHit.Location.Z - CenterZ);
                    if (HeightDiff <= HeightTolerance)
                    {
                        ++SimilarPointsCount;
                        HeightSum += CornerHit.Location.Z;
                    }
                }
            }

            if (SimilarPointsCount < (MinRequiredSimilarCorners + 1))
            {
                continue;
            }

            const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CenterHit.ImpactNormal, FVector::UpVector)));
            if (SlopeAngle > MaxWalkableSlopeAngle)
            {
                continue;
            }

            FHexCellData CellData;
            CellData.Coordinate = CurrentCoord;
            CellData.bIsPassable = true;
            const float AverageZ = HeightSum / static_cast<float>(SimilarPointsCount);
            CellData.WorldLocation = FVector(CenterHit.Location.X, CenterHit.Location.Y, AverageZ + GroundOffset);

            GeneratedGridData.Add(CurrentCoord, CellData);
        }
    }

    GridData = GeneratedGridData;

    TargetGridDataAsset->Modify();
    TargetGridDataAsset->GridData.Empty();
    TargetGridDataAsset->HexSize = HexSize;
    TargetGridDataAsset->GridYaw = BoundsTransform.GetRotation().Rotator().Yaw;
    TargetGridDataAsset->DecalYawOffset = DecalYawOffset;

    for (const TPair<FHexCoord, FHexCellData>& CellPair : GeneratedGridData)
    {
        const FHexCoord& Coord = CellPair.Key;
        TargetGridDataAsset->GridData.Add(FVector2D(Coord.Q, Coord.R), CellPair.Value);
    }

    TargetGridDataAsset->MarkPackageDirty();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("GenerateGrid saved %d cells to %s."),
        TargetGridDataAsset->GridData.Num(),
        *TargetGridDataAsset->GetName()
    );
}
