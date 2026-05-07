# Hex Battle Config Architecture

## Summary

这份笔记用于整理六边形战棋项目里与关卡、地图、棋子配置相关的推荐架构。

核心原则是把以下内容分开：

- 地图静态数据
- 棋子模板数据
- 战斗配置数据
- 运行时状态数据

这样可以支持：

- 多个关卡
- 一个关卡对应多种战斗配置
- 策划方便配置
- 程序方便查询与扩展

## 1. 数据分层

推荐拆成 4 层。

### 1.1 地图层：`UHexGridDataAsset`

职责：描述地图本身的静态网格数据。

适合存放：

- 格子坐标
- 格子世界坐标
- 是否可通行
- 地形类型
- 移动消耗
- 其他不会在战斗中频繁变化的地图属性

不适合存放：

- 当前格子上站了哪个棋子
- 当前高亮状态
- 战斗过程中的临时障碍

一句话理解：

`UHexGridDataAsset` 负责描述“这张地图长什么样”。

### 1.2 棋子模板层：`UChessPieceDataAsset`

职责：描述某一种棋子的模板配置。

适合存放：

- 棋子名字
- 模型或蓝图类
- 最大生命值
- 移动力
- 攻击范围
- 阵营类型
- 技能配置
- 图标、立绘、说明文本

一句话理解：

`UChessPieceDataAsset` 负责描述“这个棋子是什么”。

### 1.3 战斗配置层：`UHexBattleSetupDataAsset`

职责：描述一场具体战斗如何开始。

适合存放：

- 本场战斗使用哪张地图
- 玩家初始单位列表
- 敌人初始单位列表
- 每个单位出生在哪个格子
- 胜利条件
- 失败条件
- 回合上限
- 特殊规则

一句话理解：

`UHexBattleSetupDataAsset` 负责描述“在这张地图上，如何开这一局战斗”。

### 1.4 运行时层：`AHexGridMap` / `ABattleManager` / 棋子 Actor

职责：管理战斗开始后会变化的状态。

例如：

- 当前格子占用关系
- 当前轮到谁行动
- 棋子移动后的位置
- 棋子死亡或召唤
- 临时 Buff / Debuff

一句话理解：

运行时对象负责“这一局现在打成什么样”。

## 2. 推荐资产关系

推荐关系如下：

- `UHexGridDataAsset`
  - 一张地图的静态网格
- `UChessPieceDataAsset`
  - 一种棋子的模板定义
- `UHexBattleSetupDataAsset`
  - 一场战斗的配置，引用地图与棋子摆放
- `AAHexGridMap`
  - 运行时地图实例，从 `UHexGridDataAsset` 读取网格
- `ABattleManager`
  - 运行时战斗入口，读取 `UHexBattleSetupDataAsset` 并生成棋子

推荐依赖方向：

- `UHexBattleSetupDataAsset` 引用 `UHexGridDataAsset`
- `UHexBattleSetupDataAsset` 的摆放项引用 `UChessPieceDataAsset`
- `ABattleManager` 持有 `UHexBattleSetupDataAsset`
- `ABattleManager` 驱动 `AAHexGridMap`

## 3. 多个关卡与多套战斗配置

这是最关键的设计点。

不要把“地图”和“战斗配置”绑死。

推荐做法是：

- 一张地图可以对应多个战斗配置
- 每个战斗配置都单独是一个 `UHexBattleSetupDataAsset`

例如：

- `DA_Grid_Forest01`
- `DA_Battle_Forest01_Story`
- `DA_Battle_Forest01_Challenge`
- `DA_Battle_Forest01_Boss`

这样可以实现：

- 同一张地图复用多次
- 一个关卡有主线战斗、挑战战斗、Boss 战斗
- 地图修改与战斗配置修改互不干扰

## 4. 策划友好的配置形式

对于战斗配置，推荐优先使用 `DataAsset + USTRUCT 数组`。

原因：

- Unreal 原生支持良好
- 能直接引用类和资产
- 结构清晰，适合复杂配置
- 比 `DataTable` 更适合带对象引用和类型引用的内容

推荐最小结构如下：

```cpp
USTRUCT(BlueprintType)
struct FHexPieceSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FHexCoord Coord;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> PieceClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UChessPieceDataAsset* PieceData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 TeamId = 0;
};
```

```cpp
UCLASS(BlueprintType)
class UHexBattleSetupDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UHexGridDataAsset* GridData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FHexPieceSpawnConfig> InitialPieces;
};
```

## 5. 运行时占用关系不要写回配置

有一个非常重要的区分：

- 策划配置的是“初始摆放”
- 运行时维护的是“当前占用”

所以不要把“当前某个格子上站的是谁”写进 `UHexGridDataAsset` 或 `UHexBattleSetupDataAsset`。

更推荐在 `AAHexGridMap` 里维护运行时占用表，例如：

```cpp
TMap<FHexCoord, AActor*> OccupiedPieces;
```

如果后面有专门的棋子类，也可以改成：

```cpp
TMap<FHexCoord, AChessPiece*> OccupiedPieces;
```

运行时应由地图或战斗管理器提供这类接口：

- `IsCellOccupied`
- `GetPieceAtCoord`
- `PlacePieceAtCoord`
- `MovePieceToCoord`
- `RemovePieceAtCoord`

## 6. 推荐运行流程

推荐启动流程如下：

1. 进入战斗时，由 `ABattleManager` 选定一个 `UHexBattleSetupDataAsset`
2. 从 `BattleSetup` 中读取 `GridData`
3. 让 `AAHexGridMap` 加载对应 `UHexGridDataAsset`
4. 遍历 `InitialPieces`
5. 根据出生坐标生成棋子 Actor
6. 将棋子注册到 `AAHexGridMap` 的占用表
7. 战斗开始后，位置变化只改运行时状态，不改配置资产

## 7. 推荐命名方式

建议统一命名，方便策划和程序协作。

地图资产：

- `DA_Grid_Forest01`
- `DA_Grid_Castle02`

棋子模板：

- `DA_Piece_Warrior`
- `DA_Piece_Archer`
- `DA_Piece_Mage`

战斗配置：

- `DA_Battle_Forest01_Story`
- `DA_Battle_Forest01_Elite`
- `DA_Battle_Forest01_Boss`

这样可以一眼看出：

- 地图资源是什么
- 棋子模板是什么
- 某场战斗对应哪张地图

## 8. 当前项目的建议落地顺序

为了减少返工，建议按下面顺序逐步推进：

1. 保持现有 `UHexGridDataAsset` 作为地图静态数据
2. 使用 `AAHexGridMap` 作为运行时地图读取入口
3. 新建 `UChessPieceDataAsset`，管理棋子模板
4. 新建 `UHexBattleSetupDataAsset`，管理战斗初始摆放
5. 给 `AAHexGridMap` 增加格子占用查询与放置接口
6. 新建 `ABattleManager` 负责读取战斗配置并生成棋子

## 9. 一句话结论

推荐架构如下：

- 地图用 `UHexGridDataAsset`
- 棋子模板用 `UChessPieceDataAsset`
- 战斗配置用 `UHexBattleSetupDataAsset`
- 运行时地图用 `AAHexGridMap`
- 运行时战斗流程用 `ABattleManager`

最重要的原则是：

静态配置和运行时状态必须分离。
