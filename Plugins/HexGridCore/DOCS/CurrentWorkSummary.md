# HexagonalGridGame Current Work Summary

更新时间：2026-05-25

## 1. 当前阶段概览

项目目前正在从“基础六边形网格与单位生成”推进到“可交互的战棋回合体验”。

当前主线可以概括为：

- 使用 `UHexGridSubsystem` 作为运行时网格入口，负责读取地图数据、生成格子视觉、路径查询和格子 Decal 控制。
- 使用 `UPawnManagerSubsystem` 根据战斗配置在网格上生成单位。
- 使用 `UTurnManagerSubsystem` 管理战斗准备、战斗开始、当前行动单位切换。
- 使用 `AHexagonalGamePlayerController` 处理玩家输入、相机控制、当前单位聚焦，以及正在实现的格子预览交互。

最近的工作重点是：在玩家回合中显示当前单位的移动范围，并在鼠标悬停格子时显示高亮反馈。

## 2. 已有核心模块

### 2.1 HexGridCore 插件

`HexGridCore` 是网格相关能力的主要承载位置。

当前包含的核心内容：

- `UHexGridDataAsset`
  - 保存地图静态格子数据。
  - 适合存储坐标、世界位置、可通行状态等不会频繁变化的数据。
- `UHexGridSubsystem`
  - 运行时网格系统。
  - 负责从 DataAsset 加载格子。
  - 负责构建格子视觉。
  - 提供路径查询、格子查询、Decal 颜色和显隐控制。
- `FHexCoord` / `UHexMathLibrary`
  - 六边形坐标与邻居计算等基础工具。
- `AHexUnitBase`
  - 网格单位基类，目前已处理单位 Mesh 不接收 Decal，避免地面高亮投射到角色模型上。

### 2.2 HexoGame 游戏模块

`HexoGame` 是当前战斗流程和玩家控制逻辑的主要位置。

当前包含的核心内容：

- `AHexagonalGameMode`
  - 战斗初始化入口。
  - 从 `AHexBattleWorldSettings` 读取地图与视觉配置。
  - 初始化 `UHexGridSubsystem`、`UPawnManagerSubsystem`、`UTurnManagerSubsystem`。
  - 等待 PlayerController 与各 Subsystem 准备完成后启动战斗。
- `AHexagonalGamePlayerController`
  - 处理增强输入。
  - 支持相机边缘滚动、缩放、环绕。
  - 监听当前行动单位变化。
  - 当前正在扩展鼠标悬停格子与移动范围预览。
- `AHexBattleUnit`
  - 战斗单位 Actor。
  - 初始化 GAS 相关组件和单位定义。
  - 当前临时提供 `GetMoveRange()`，返回固定移动范围 `4`。
- `UBattleConfigDataAsset` / `UBattleUnitDefinition`
  - 用于描述战斗配置与单位定义。

## 3. 当前未提交改动整理

当前工作区存在以下未提交改动。

### 3.1 移动范围查询

文件：

- `Plugins/HexGridCore/Source/HexGridCore/Public/Subsystem/HexGridSubsystem.h`
- `Plugins/HexGridCore/Source/HexGridCore/Private/Subsystem/HexGridSubsystem.cpp`

新增能力：

- `GetReachableCells(const FHexCoord& StartCoord, int32 MoveRange) const`

当前实现逻辑：

- 从起点格子开始。
- 如果起点不存在或不可通行，直接返回空数组。
- 使用 BFS 向外扩展。
- 通过 `CanMoveBetweenCells(CurrentCoord, Neighbor)` 判断相邻格是否可移动。
- 不把起点自身加入结果。
- 返回移动范围内所有可达格子。

用途：

- 给 PlayerController 显示当前单位的可移动格子范围。

### 3.2 Decal 显隐控制

文件：

- `Plugins/HexGridCore/Source/HexGridCore/Public/Subsystem/HexGridSubsystem.h`
- `Plugins/HexGridCore/Source/HexGridCore/Private/Subsystem/HexGridSubsystem.cpp`

新增能力：

- `SetDecalVisible(const FHexCoord& Coord, bool bVisible)`
- `HideAllDecals()`

当前用途：

- 地图视觉构建后先隐藏所有格子 Decal。
- 只有在移动范围预览或鼠标悬停时显示对应格子的 Decal。

相关调用：

- `AHexagonalGameMode::InitialSubsystem()` 在 `BuildGridVisuals()` 后调用 `HideAllDecals()`。

### 3.3 当前单位移动范围预览

文件：

- `Source/HexoGame/Public/Player/HexagonalGamePlayerController.h`
- `Source/HexoGame/Private/Player/HexagonalGamePlayerController.cpp`

新增状态：

- `MovementRangeColor`
- `CurrentMovementRangeCells`

新增逻辑：

- `OnCurrentUnitChanged()` 切换当前行动单位时：
  - 清除旧悬停格。
  - 清除旧移动范围预览。
  - 聚焦相机到新单位。
  - 显示新单位移动范围。
- `ShowMovementRangeForUnit()`：
  - 读取单位 `GetMoveRange()`。
  - 调用 `UHexGridSubsystem::GetReachableCells()`。
  - 将可达格子 Decal 设置为移动范围颜色并显示。
- `ClearMovementRangePreview()`：
  - 隐藏旧移动范围格子的 Decal。
  - 重置旧格子 Decal 颜色。

### 3.4 鼠标悬停格子高亮

文件：

- `Source/HexoGame/Public/Player/HexagonalGamePlayerController.h`
- `Source/HexoGame/Private/Player/HexagonalGamePlayerController.cpp`

新增状态：

- `bHoveredCell`
- `CurrentHoveredCell`

新增逻辑：

- `Tick()` 每帧调用 `UpdateHoveredCell()`。
- `UpdateHoveredCell()`：
  - 使用 `GetHitResultUnderCursor(ECC_Visibility, false, HitResult)` 获取鼠标命中的世界位置。
  - 通过 `UHexGridSubsystem::GetCellAtWorldLocation()` 转成格子坐标。
  - 如果悬停格发生变化，先恢复旧格子视觉，再把新格子设置为黄色高亮并显示 Decal。
- `RestoreCellVisual()`：
  - 如果格子属于当前移动范围，则恢复为移动范围颜色并保持显示。
  - 否则隐藏 Decal 并重置颜色。

### 3.5 单位移动范围临时值

文件：

- `Source/HexoGame/Public/Pawn/HexBattleUnit.h`
- `Source/HexoGame/Private/Pawn/HexBattleUnit.cpp`

新增能力：

- `GetMoveRange() const`

当前状态：

- 目前返回固定值 `4`。
- 注释中已标记为占位逻辑，后续应改为从单位定义、属性系统或 Gameplay Attribute 中读取。

### 3.6 单位不接收 Decal

文件：

- `Plugins/HexGridCore/Source/HexGridCore/Private/Units/HexUnitBase.cpp`

新增行为：

- `Mesh->SetReceivesDecals(false);`

目的：

- 避免格子 Decal 高亮投射到单位 Mesh 上，保证高亮只表现为地面反馈。

### 3.7 蓝图资源改动

文件：

- `Content/Blueprints/Player/BP_CameraPawn.uasset`

当前只知道该资源存在未提交修改。

建议在提交前通过 Unreal Editor 确认：

- 相机 Pawn 输入或默认参数是否被调整。
- 是否与最近的 Zoom / Orbit / Edge Scroll 行为有关。
- 是否需要和本轮 C++ 改动一起提交。

## 4. 当前运行流程

当前战斗启动流程大致如下：

1. `AHexagonalGameMode::BeginPlay()`
2. 调用 `InitialSubsystem()`
3. 从 `AHexBattleWorldSettings` 获取地图与 Decal 材质配置。
4. `UHexGridSubsystem` 加载地图 DataAsset。
5. 设置 Decal 材质。
6. 构建网格视觉。
7. 隐藏所有 Decal。
8. `UPawnManagerSubsystem` 根据 `TestBattleConfig` 与 `UnitClass` 生成单位。
9. `UTurnManagerSubsystem` 准备战斗。
10. PlayerController 准备完成后通知 GameMode。
11. GameMode 检查所有条件后调用 `TurnManagerSubsystem->StartBattle()`。
12. 当前行动单位变化时，PlayerController 聚焦单位并显示移动范围。
13. 每帧根据鼠标位置更新悬停格高亮。

## 5. 已知待确认问题

### 5.1 移动范围是否应考虑单位占用

当前 `GetReachableCells()` 主要依赖格子通行和 `CanMoveBetweenCells()`。

后续需要确认：

- 敌我单位占据的格子是否阻挡移动。
- 友方单位占据的格子是否可穿过但不可停留。
- 敌方单位占据的格子是否完全阻挡。
- 移动范围预览是否应该排除当前已有单位的格子。

### 5.2 MoveRange 数据来源

当前 `AHexBattleUnit::GetMoveRange()` 返回固定值 `4`。

后续候选来源：

- `UBattleUnitDefinition`
- GAS Attribute，例如 MoveRange / Mobility
- GameplayEffect 初始化后的当前属性值

建议尽早统一来源，避免后续移动、AI、UI 显示各自读取不同数据。

### 5.3 Decal 颜色与状态管理

当前 Decal 同时承担：

- 移动范围预览
- 鼠标悬停高亮

后续如果继续增加攻击范围、技能范围、不可选区域等状态，建议考虑统一的格子预览状态系统。

可能需要支持优先级：

- Hover 高于 MoveRange
- AttackRange 高于 MoveRange
- Blocked / InvalidMove 用单独颜色

### 5.4 鼠标命中通道

当前悬停检测使用 `ECC_Visibility`。

后续需要确认：

- 格子视觉组件是否稳定响应 Visibility Trace。
- 单位 Mesh 是否会挡住鼠标命中，导致无法选中脚下格子。
- 是否需要单独的 Trace Channel，例如 `HexGridTrace`。

### 5.5 清理未使用状态

`UHexGridSubsystem.h` 当前新增了：

- `bHasHoveredCoord`
- `HoveredCoord`

但当前悬停状态实际由 `AHexagonalGamePlayerController` 维护。

如果后续不打算把 Hover 状态下放到 GridSubsystem，可以删除这两个字段，减少状态重复。

## 6. 建议下一步

推荐按下面顺序推进：

1. 编译验证当前 C++ 改动。
2. 在 Editor 中确认：
   - 地图初始 Decal 是否全部隐藏。
   - 当前单位切换时是否显示蓝色移动范围。
   - 鼠标悬停格子是否显示黄色高亮。
   - 鼠标离开格子后，移动范围格子是否恢复蓝色，普通格子是否隐藏。
3. 清理 `UHexGridSubsystem` 中未使用的 Hover 字段。
4. 将 `GetMoveRange()` 从固定值改为读取单位定义或 GAS 属性。
5. 给格子占用关系补充移动范围规则。
6. 将移动范围预览和悬停高亮抽象成更明确的格子预览状态，方便后续扩展攻击范围和技能范围。

## 7. 当前提交建议

如果本轮验证通过，可以把当前改动整理为一次提交：

提交主题建议：

```text
Add movement range and hover cell preview
```

提交内容建议包含：

- `UHexGridSubsystem` 可达格查询。
- Grid Decal 显隐接口。
- GameMode 初始化后隐藏所有 Decal。
- PlayerController 当前单位移动范围预览。
- PlayerController 鼠标悬停格子高亮。
- 单位 Mesh 禁用接收 Decal。
- 如果 `BP_CameraPawn` 的改动确实与相机控制相关，也可以一起提交；否则建议拆开。

