from __future__ import annotations

import heapq
import math
import tkinter as tk
from dataclasses import dataclass
from tkinter import ttk
from typing import Dict, List, Optional, Set, Tuple

from hex_qr_visualizer import AxialCoord, HexGridModel, build_demo_grid


@dataclass(slots=True)
class AStarStep:
    step_index: int
    action: str
    current: Optional[AxialCoord]
    neighbor: Optional[AxialCoord]
    tentative_g: Optional[int]
    open_nodes: Set[AxialCoord]
    closed_nodes: Set[AxialCoord]
    path_nodes: Set[AxialCoord]
    path: List[AxialCoord]
    message: str


def hex_distance(a: AxialCoord, b: AxialCoord) -> int:
    return max(abs(a.q - b.q), abs(a.r - b.r), abs(a.s - b.s))


def reconstruct_path(
    came_from: Dict[AxialCoord, AxialCoord],
    current: AxialCoord,
) -> List[AxialCoord]:
    path = [current]
    while current in came_from:
        current = came_from[current]
        path.append(current)
    path.reverse()
    return path


def build_a_star_trace(
    model: HexGridModel,
    start: AxialCoord,
    goal: AxialCoord,
) -> List[AStarStep]:
    steps: List[AStarStep] = []

    def record(
        action: str,
        current: Optional[AxialCoord],
        neighbor: Optional[AxialCoord],
        tentative_g: Optional[int],
        open_heap: List[Tuple[int, int, AxialCoord]],
        closed_set: Set[AxialCoord],
        came_from: Dict[AxialCoord, AxialCoord],
        message: str,
        path: Optional[List[AxialCoord]] = None,
    ) -> None:
        open_nodes = {coord for _, _, coord in open_heap if coord not in closed_set}
        final_path = path if path is not None else []
        steps.append(
            AStarStep(
                step_index=len(steps),
                action=action,
                current=current,
                neighbor=neighbor,
                tentative_g=tentative_g,
                open_nodes=open_nodes,
                closed_nodes=set(closed_set),
                path_nodes=set(final_path),
                path=list(final_path),
                message=message,
            )
        )

    start_cell = model.get_cell(start)
    goal_cell = model.get_cell(goal)
    if start_cell is None or goal_cell is None:
        record(
            action="invalid",
            current=None,
            neighbor=None,
            tentative_g=None,
            open_heap=[],
            closed_set=set(),
            came_from={},
            message="起点或终点不存在，无法开始 A*。",
        )
        return steps

    if not start_cell.walkable or not goal_cell.walkable:
        record(
            action="invalid",
            current=None,
            neighbor=None,
            tentative_g=None,
            open_heap=[],
            closed_set=set(),
            came_from={},
            message="起点或终点不可通行，无法开始 A*。",
        )
        return steps

    open_heap: List[Tuple[int, int, AxialCoord]] = []
    counter = 0
    came_from: Dict[AxialCoord, AxialCoord] = {}
    g_score: Dict[AxialCoord, int] = {start: 0}
    closed_set: Set[AxialCoord] = set()

    heapq.heappush(open_heap, (hex_distance(start, goal), counter, start))
    record(
        action="init",
        current=start,
        neighbor=None,
        tentative_g=0,
        open_heap=open_heap,
        closed_set=closed_set,
        came_from=came_from,
        message="初始化：把起点放入 open heap。",
    )

    while open_heap:
        _, _, current = heapq.heappop(open_heap)
        if current in closed_set:
            record(
                action="skip_closed",
                current=current,
                neighbor=None,
                tentative_g=None,
                open_heap=open_heap,
                closed_set=closed_set,
                came_from=came_from,
                message=f"跳过 {current.q},{current.r}，因为它已经在 closed set 里。",
            )
            continue

        record(
            action="pop_current",
            current=current,
            neighbor=None,
            tentative_g=g_score[current],
            open_heap=open_heap,
            closed_set=closed_set,
            came_from=came_from,
            message=(
                f"从 open heap 取出当前格子 {current.q},{current.r}。"
                f" g={g_score[current]} h={hex_distance(current, goal)}"
                f" f={g_score[current] + hex_distance(current, goal)}"
            ),
        )

        if current == goal:
            final_path = reconstruct_path(came_from, current)
            record(
                action="goal_reached",
                current=current,
                neighbor=None,
                tentative_g=g_score[current],
                open_heap=open_heap,
                closed_set=closed_set,
                came_from=came_from,
                message=f"到达终点，路径长度为 {len(final_path)}。",
                path=final_path,
            )
            return steps

        closed_set.add(current)
        record(
            action="close_current",
            current=current,
            neighbor=None,
            tentative_g=g_score[current],
            open_heap=open_heap,
            closed_set=closed_set,
            came_from=came_from,
            message=f"把 {current.q},{current.r} 放入 closed set。",
        )

        for neighbor_cell in model.get_neighbors(current, walkable_only=True):
            neighbor = neighbor_cell.coord
            tentative_g = g_score[current] + 1

            if neighbor in closed_set:
                record(
                    action="neighbor_closed",
                    current=current,
                    neighbor=neighbor,
                    tentative_g=tentative_g,
                    open_heap=open_heap,
                    closed_set=closed_set,
                    came_from=came_from,
                    message=(
                        f"检查邻居 {neighbor.q},{neighbor.r}。"
                        f" tentative_g={tentative_g}，但它已在 closed set 中，跳过。"
                    ),
                )
                continue

            old_g = g_score.get(neighbor, float("inf"))
            if tentative_g < old_g:
                came_from[neighbor] = current
                g_score[neighbor] = tentative_g
                counter += 1
                f_score = tentative_g + hex_distance(neighbor, goal)
                heapq.heappush(open_heap, (f_score, counter, neighbor))
                record(
                    action="update_neighbor",
                    current=current,
                    neighbor=neighbor,
                    tentative_g=tentative_g,
                    open_heap=open_heap,
                    closed_set=closed_set,
                    came_from=came_from,
                    message=(
                        f"更新邻居 {neighbor.q},{neighbor.r}。"
                        f" tentative_g={tentative_g} 比旧值 {old_g} 更优，"
                        f"因此写入 came_from / g_score，并压入 open heap。"
                    ),
                )
            else:
                record(
                    action="reject_neighbor",
                    current=current,
                    neighbor=neighbor,
                    tentative_g=tentative_g,
                    open_heap=open_heap,
                    closed_set=closed_set,
                    came_from=came_from,
                    message=(
                        f"检查邻居 {neighbor.q},{neighbor.r}。"
                        f" tentative_g={tentative_g} 不优于旧值 {old_g}，不更新。"
                    ),
                )

    record(
        action="no_path",
        current=None,
        neighbor=None,
        tentative_g=None,
        open_heap=open_heap,
        closed_set=closed_set,
        came_from={},
        message="open heap 已空，仍未到达终点，因此不存在可通行路径。",
    )
    return steps


class AStarTraceVisualizer:
    def __init__(self, model: HexGridModel, hex_size: float = 36.0) -> None:
        self.model = model
        self.hex_size = hex_size
        self.start = AxialCoord(0, 0)
        self.goal = AxialCoord(5, 4)
        self.steps = build_a_star_trace(self.model, self.start, self.goal)
        self.current_step_index = 0
        self.auto_play_job: Optional[str] = None

        self.root = tk.Tk()
        self.root.title("Hex A* Step Demo")
        self.root.geometry("1440x900")

        self.main_frame = ttk.Frame(self.root, padding=12)
        self.main_frame.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.main_frame, bg="#111827", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.side_panel = ttk.Frame(self.main_frame, width=360)
        self.side_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(12, 0))

        self.step_text = tk.StringVar()
        self.action_text = tk.StringVar()
        self.current_text = tk.StringVar()
        self.neighbor_text = tk.StringVar()
        self.tentative_text = tk.StringVar()
        self.message_text = tk.StringVar()

        self._build_side_panel()
        self.canvas.bind("<Configure>", lambda _event: self.redraw())
        self._sync_step_labels()
        self.redraw()

    def _build_side_panel(self) -> None:
        ttk.Label(self.side_panel, text="A* 逐步演示", font=("Segoe UI", 16, "bold")).pack(anchor=tk.W)
        ttk.Label(
            self.side_panel,
            text="颜色说明：蓝=起点，紫=终点，红=当前节点，青=open，橙=closed，黄=最终路径。",
            justify=tk.LEFT,
            wraplength=330,
        ).pack(anchor=tk.W, pady=(6, 12))

        controls = ttk.LabelFrame(self.side_panel, text="控制", padding=10)
        controls.pack(fill=tk.X)
        ttk.Button(controls, text="重置", command=self.reset_demo).pack(fill=tk.X)
        ttk.Button(controls, text="下一步", command=self.next_step).pack(fill=tk.X, pady=(8, 0))
        ttk.Button(controls, text="自动播放", command=self.toggle_auto_play).pack(fill=tk.X, pady=(8, 0))

        info = ttk.LabelFrame(self.side_panel, text="当前步骤", padding=10)
        info.pack(fill=tk.X, pady=(12, 0))
        ttk.Label(info, textvariable=self.step_text, justify=tk.LEFT).pack(anchor=tk.W)
        ttk.Label(info, textvariable=self.action_text, justify=tk.LEFT).pack(anchor=tk.W, pady=(6, 0))
        ttk.Label(info, textvariable=self.current_text, justify=tk.LEFT).pack(anchor=tk.W, pady=(6, 0))
        ttk.Label(info, textvariable=self.neighbor_text, justify=tk.LEFT).pack(anchor=tk.W, pady=(6, 0))
        ttk.Label(info, textvariable=self.tentative_text, justify=tk.LEFT).pack(anchor=tk.W, pady=(6, 0))

        explain = ttk.LabelFrame(self.side_panel, text="解释", padding=10)
        explain.pack(fill=tk.BOTH, expand=True, pady=(12, 0))
        ttk.Label(
            explain,
            textvariable=self.message_text,
            justify=tk.LEFT,
            wraplength=320,
        ).pack(anchor=tk.NW, fill=tk.X)

    def run(self) -> None:
        self.root.mainloop()

    def reset_demo(self) -> None:
        self._cancel_auto_play()
        self.current_step_index = 0
        self._sync_step_labels()
        self.redraw()

    def next_step(self) -> None:
        if self.current_step_index < len(self.steps) - 1:
            self.current_step_index += 1
            self._sync_step_labels()
            self.redraw()
        else:
            self._cancel_auto_play()

    def toggle_auto_play(self) -> None:
        if self.auto_play_job is not None:
            self._cancel_auto_play()
            return
        self._auto_play_step()

    def _auto_play_step(self) -> None:
        self.next_step()
        if self.current_step_index < len(self.steps) - 1:
            self.auto_play_job = self.root.after(750, self._auto_play_step)
        else:
            self.auto_play_job = None

    def _cancel_auto_play(self) -> None:
        if self.auto_play_job is not None:
            self.root.after_cancel(self.auto_play_job)
            self.auto_play_job = None

    def _sync_step_labels(self) -> None:
        step = self.steps[self.current_step_index]
        self.step_text.set(f"Step: {step.step_index + 1} / {len(self.steps)}")
        self.action_text.set(f"Action: {step.action}")
        self.current_text.set(self._format_coord("Current", step.current))
        self.neighbor_text.set(self._format_coord("Neighbor", step.neighbor))
        if step.tentative_g is None:
            self.tentative_text.set("tentative_g: -")
        else:
            self.tentative_text.set(f"tentative_g: {step.tentative_g}")
        self.message_text.set(step.message)

    @staticmethod
    def _format_coord(label: str, coord: Optional[AxialCoord]) -> str:
        if coord is None:
            return f"{label}: -"
        return f"{label}: Q={coord.q}, R={coord.r}, S={coord.s}"

    def redraw(self) -> None:
        self.canvas.delete("all")
        if not self.model.cells:
            return

        step = self.steps[self.current_step_index]
        centers = [self.axial_to_pixel(coord) for coord in self.model.cells]
        min_x = min(point[0] for point in centers) - self.hex_size * 1.4
        max_x = max(point[0] for point in centers) + self.hex_size * 1.4
        min_y = min(point[1] for point in centers) - self.hex_size * 1.4
        max_y = max(point[1] for point in centers) + self.hex_size * 1.4

        canvas_width = max(self.canvas.winfo_width(), 400)
        canvas_height = max(self.canvas.winfo_height(), 400)
        offset_x = (canvas_width - (max_x - min_x)) * 0.5 - min_x
        offset_y = (canvas_height - (max_y - min_y)) * 0.5 - min_y

        for coord, cell in self.model.cells.items():
            center_x, center_y = self.axial_to_pixel(coord)
            center_x += offset_x
            center_y += offset_y
            points = self.hex_points(center_x, center_y)

            fill = self.resolve_cell_color(coord, cell, step)
            outline = "#f8fafc" if coord == step.current else "#374151"
            width = 3 if coord == step.current else 1

            self.canvas.create_polygon(points, fill=fill, outline=outline, width=width)

            label = f"{coord.q},{coord.r}"
            if not cell.walkable:
                label += "\nX"
            self.canvas.create_text(
                center_x,
                center_y,
                text=label,
                fill="#e5e7eb",
                font=("Consolas", 9, "bold"),
                justify=tk.CENTER,
            )

    def axial_to_pixel(self, coord: AxialCoord) -> Tuple[float, float]:
        x = self.hex_size * math.sqrt(3) * (coord.q + coord.r / 2.0)
        y = self.hex_size * 1.5 * coord.r
        return x, y

    def hex_points(self, center_x: float, center_y: float) -> List[float]:
        points: List[float] = []
        for index in range(6):
            angle_deg = 60 * index - 30
            angle_rad = math.radians(angle_deg)
            points.append(center_x + self.hex_size * math.cos(angle_rad))
            points.append(center_y + self.hex_size * math.sin(angle_rad))
        return points

    def resolve_cell_color(self, coord: AxialCoord, cell, step: AStarStep) -> str:
        if coord in step.path_nodes:
            return "#facc15"
        if coord == step.neighbor:
            return "#22c55e"
        if coord == step.current:
            return "#ef4444"
        if coord == self.goal:
            return "#8b5cf6"
        if coord == self.start:
            return "#3b82f6"
        if coord in step.closed_nodes:
            return "#f59e0b"
        if coord in step.open_nodes:
            return "#06b6d4"
        if not cell.walkable:
            return "#4b5563"
        return "#334155"


def main() -> None:
    model = build_demo_grid()
    app = AStarTraceVisualizer(model=model)
    app.run()


if __name__ == "__main__":
    main()
