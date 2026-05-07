from __future__ import annotations
from queue import Queue
import math
import heapq
import random
import tkinter as tk
from dataclasses import dataclass, field
from tkinter import ttk
from typing import Dict, Iterable, List, Optional, Tuple


@dataclass(frozen=True, slots=True)
class AxialCoord:
    q: int
    r: int

    @property
    def s(self) -> int:
        return -self.q - self.r


@dataclass(slots=True)
class HexCell:
    coord: AxialCoord
    height: int = 0
    walkable: bool = True
    color_override: Optional[str] = None


@dataclass(slots=True)
class HexGridModel:
    width: int
    height: int
    cells: Dict[AxialCoord, HexCell] = field(init=False, default_factory=dict)
    MAX_STEP_HEIGHT_DIFF: int = 2

    DIRECTION_OFFSETS: Tuple[AxialCoord, ...] = (
        AxialCoord(1, 0),
        AxialCoord(1, -1),
        AxialCoord(0, -1),
        AxialCoord(-1, 0),
        AxialCoord(-1, 1),
        AxialCoord(0, 1),
    )

    def __post_init__(self) -> None:
        for q in range(self.width):
            for r in range(self.height):
                coord = AxialCoord(q, r)
                self.cells[coord] = HexCell(coord=coord)

    def get_cell(self, coord: AxialCoord) -> Optional[HexCell]:
        return self.cells.get(coord)

    def set_height(self, coord: AxialCoord, height: int) -> None:
        cell = self.require_cell(coord)
        cell.height = height

    def set_walkable(self, coord: AxialCoord, walkable: bool) -> None:
        cell = self.require_cell(coord)
        cell.walkable = walkable

    def set_color(self, coord: AxialCoord, color: Optional[str]) -> None:
        cell = self.require_cell(coord)
        cell.color_override = color

    def color_cells(self, coords: Iterable[AxialCoord | Tuple[int, int]], color: str) -> None:
        for item in coords:
            coord = self._normalize_coord(item)
            if coord in self.cells:
                self.cells[coord].color_override = color

    def clear_all_colors(self) -> None:
        for cell in self.cells.values():
            cell.color_override = None



    def get_neighbors(self, coord: AxialCoord, walkable_only: bool = False) -> List[HexCell]:
        neighbors: List[HexCell] = []
        for offset in self.DIRECTION_OFFSETS:
            target = AxialCoord(coord.q + offset.q, coord.r + offset.r)
            cell = self.cells.get(target)
            if cell is None:
                continue
            if walkable_only and not cell.walkable:
                continue
            neighbors.append(cell)
        return neighbors

    def require_cell(self, coord: AxialCoord) -> HexCell:
        cell = self.get_cell(coord)
        if cell is None:
            raise KeyError(f"Cell {coord} does not exist in the grid.")
        return cell

    def is_walkable_coord(self, coord: AxialCoord) -> bool:
        cell = self.get_cell(coord)
        return cell is not None and cell.walkable

    def can_step_to(
        self,
        current: AxialCoord,
        neighbor: AxialCoord,
        use_height_rule: bool = True,
    ) -> bool:
        current_cell = self.get_cell(current)
        neighbor_cell = self.get_cell(neighbor)
        if current_cell is None or neighbor_cell is None:
            return False
        if not neighbor_cell.walkable:
            return False
        if not use_height_rule:
            return True
        return abs(current_cell.height - neighbor_cell.height) <= self.MAX_STEP_HEIGHT_DIFF

    def _validate_path_endpoints(
        self,
        start: AxialCoord,
        end: AxialCoord,
        use_height_rule: bool,
    ) -> bool:
        if not self.is_walkable_coord(start) or not self.is_walkable_coord(end):
            return False
        if not use_height_rule:
            return True
        return self.get_cell(start) is not None and self.get_cell(end) is not None

    def _find_path_bfs_internal(
        self,
        start: AxialCoord,
        end: AxialCoord,
        use_height_rule: bool,
    ) -> List[AxialCoord]:
        if not self._validate_path_endpoints(start, end, use_height_rule):
            return []
        if start == end:
            return [start]

        visited = {start}
        queue: Queue[Tuple[AxialCoord, List[AxialCoord]]] = Queue()
        queue.put((start, [start]))

        while not queue.empty():
            current, path = queue.get()
            for neighbor in self.get_neighbors(current, walkable_only=True):
                neighbor_coord = neighbor.coord
                if neighbor_coord in visited:
                    continue
                if not self.can_step_to(current, neighbor_coord, use_height_rule=use_height_rule):
                    continue
                next_path = path + [neighbor_coord]
                if neighbor_coord == end:
                    return next_path
                visited.add(neighbor_coord)
                queue.put((neighbor_coord, next_path))

        return []

    def find_path_without_height(self, start: AxialCoord, end: AxialCoord) -> List[AxialCoord]:
        return self._find_path_bfs_internal(start, end, use_height_rule=False)

    def find_path_bfs(self, start: AxialCoord, end: AxialCoord) -> List[AxialCoord]:
        return self._find_path_bfs_internal(start, end, use_height_rule=True)
    
    @staticmethod
    def hex_distance(a: AxialCoord, b: AxialCoord) -> int:
        return max(abs(a.q - b.q), abs(a.r - b.r), abs(a.s - b.s))
    
    @staticmethod
    def reconstruct_path(came_from: Dict[AxialCoord, AxialCoord], current: AxialCoord) -> List[AxialCoord]:
        total_path = [current]
        while current in came_from:
            current = came_from[current]
            total_path.append(current)
        return total_path[::-1]  # Return reversed path
    
    def _find_path_a_star_internal(
        self,
        start: AxialCoord,
        end: AxialCoord,
        use_height_rule: bool,
    ) -> List[AxialCoord]:
        start_cell = self.get_cell(start)
        end_cell = self.get_cell(end)
        if start_cell is None or end_cell is None:
            return []
        if not start_cell.walkable or not end_cell.walkable:
            return []
        if start == end:
            return [start]
        open_heap: List[tuple[int, int, AxialCoord]] = []
        counter = 0
        came_from: Dict[AxialCoord, AxialCoord] = {}
        g_score: Dict[AxialCoord, int] = {start: 0}
        closed_set: set[AxialCoord] = set()

        start_f = self.hex_distance(start, end)
        heapq.heappush(open_heap, (start_f, counter, start))
        while open_heap:
            _, _, current = heapq.heappop(open_heap)
            if current in closed_set:
                continue
            if current == end:
                return self.reconstruct_path(came_from, current)
            
            closed_set.add(current)
            for neighbor in self.get_neighbors(current, walkable_only=True):
                neighbor_coord = neighbor.coord
                if neighbor_coord in closed_set:
                    continue
                if not self.can_step_to(current, neighbor_coord, use_height_rule=use_height_rule):
                    continue
                
                tentative_g = g_score[current] + 1
                if tentative_g < g_score.get(neighbor_coord, float("inf")):
                    came_from[neighbor_coord] = current
                    g_score[neighbor_coord] = tentative_g
                    counter += 1
                    f_score = tentative_g + self.hex_distance(neighbor_coord, end)
                    heapq.heappush(open_heap, (f_score, counter, neighbor_coord))

        return []  # No path found

    def find_path_a_star_without_height(self, start: AxialCoord, end: AxialCoord) -> List[AxialCoord]:
        return self._find_path_a_star_internal(start, end, use_height_rule=False)

    def find_path_a_star(self, start: AxialCoord, end: AxialCoord) -> List[AxialCoord]:
        return self._find_path_a_star_internal(start, end, use_height_rule=True)


    @staticmethod
    def _normalize_coord(coord: AxialCoord | Tuple[int, int]) -> AxialCoord:
        if isinstance(coord, AxialCoord):
            return coord
        return AxialCoord(coord[0], coord[1])


class HexGridVisualizer:
    def __init__(self, model: HexGridModel, hex_size: float = 34.0) -> None:
        self.model = model
        self.hex_size = hex_size
        self.selected_coord: Optional[AxialCoord] = None
        self.start_coord: Optional[AxialCoord] = None
        self.end_coord: Optional[AxialCoord] = None
        self.canvas_items: Dict[int, AxialCoord] = {}

        self.root = tk.Tk()
        self.root.title("Hex QR Grid Pre-Validation")
        self.root.geometry("1280x860")

        self.main_frame = ttk.Frame(self.root, padding=12)
        self.main_frame.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.main_frame, bg="#182028", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.side_panel = ttk.Frame(self.main_frame, width=320)
        self.side_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(12, 0))

        self.coord_text = tk.StringVar(value="未选择格子")
        self.start_text = tk.StringVar(value="Start: 未设置")
        self.end_text = tk.StringVar(value="End: 未设置")
        self.path_text = tk.StringVar(value="Path: 未设置起点和终点")
        self.height_var = tk.IntVar(value=0)
        self.walkable_var = tk.BooleanVar(value=True)
        self.algorithm_var = tk.StringVar(value="A*")
        self.use_height_rule_var = tk.BooleanVar(value=True)

        self._build_side_panel()
        self._bind_events()
        self.redraw()

    def _build_side_panel(self) -> None:
        ttk.Label(self.side_panel, text="六边形 QR 网格", font=("Segoe UI", 16, "bold")).pack(anchor=tk.W)
        ttk.Label(
            self.side_panel,
            text="左键选择格子。右侧可编辑 height / walkable。\n批量颜色接口可直接给路径结果上色。",
            justify=tk.LEFT,
        ).pack(anchor=tk.W, pady=(6, 16))

        info_frame = ttk.LabelFrame(self.side_panel, text="当前格子", padding=10)
        info_frame.pack(fill=tk.X)
        ttk.Label(info_frame, textvariable=self.coord_text).pack(anchor=tk.W)
        ttk.Label(info_frame, textvariable=self.start_text).pack(anchor=tk.W, pady=(6, 0))
        ttk.Label(info_frame, textvariable=self.end_text).pack(anchor=tk.W, pady=(2, 0))
        ttk.Label(info_frame, textvariable=self.path_text, justify=tk.LEFT).pack(anchor=tk.W, pady=(2, 0))
        ttk.Label(info_frame, text="Height").pack(anchor=tk.W, pady=(12, 0))
        height_scale = ttk.Scale(
            info_frame,
            from_=-3,
            to=8,
            orient=tk.HORIZONTAL,
            command=self._on_height_scale_changed,
        )
        height_scale.pack(fill=tk.X)
        self.height_scale = height_scale

        self.height_value_label = ttk.Label(info_frame, text="0")
        self.height_value_label.pack(anchor=tk.W, pady=(4, 8))

        walkable_check = ttk.Checkbutton(
            info_frame,
            text="可通行",
            variable=self.walkable_var,
            command=self._apply_selected_properties,
        )
        walkable_check.pack(anchor=tk.W)

        actions_frame = ttk.LabelFrame(self.side_panel, text="操作", padding=10)
        actions_frame.pack(fill=tk.X, pady=(16, 0))

        mode_frame = ttk.LabelFrame(actions_frame, text="寻路模式", padding=8)
        mode_frame.pack(fill=tk.X, pady=(0, 8))
        ttk.Radiobutton(
            mode_frame,
            text="BFS",
            value="BFS",
            variable=self.algorithm_var,
            command=self._refresh_path_highlight,
        ).pack(anchor=tk.W)
        ttk.Radiobutton(
            mode_frame,
            text="A*",
            value="A*",
            variable=self.algorithm_var,
            command=self._refresh_path_highlight,
        ).pack(anchor=tk.W)
        ttk.Checkbutton(
            mode_frame,
            text="启用 Height 规则",
            variable=self.use_height_rule_var,
            command=self._refresh_path_highlight,
        ).pack(anchor=tk.W, pady=(6, 0))

        ttk.Button(actions_frame, text="随机高度", command=self.randomize_heights).pack(fill=tk.X)
        ttk.Button(actions_frame, text="清空颜色", command=self.clear_colors).pack(fill=tk.X, pady=(8, 0))
        ttk.Button(actions_frame, text="演示路径高亮", command=self.demo_highlight_path).pack(fill=tk.X, pady=(8, 0))

        api_frame = ttk.LabelFrame(self.side_panel, text="可复用接口", padding=10)
        api_frame.pack(fill=tk.X, pady=(16, 0))
        api_text = (
            "model.set_height(AxialCoord(q, r), value)\n"
            "model.set_walkable(AxialCoord(q, r), True/False)\n"
            "model.color_cells([(0, 0), (1, 0), (2, 0)], '#ffcc33')\n"
            "model.find_path_without_height(start, end)\n"
            "model.find_path_bfs(start, end)\n"
            "model.find_path_a_star_without_height(start, end)\n"
            "model.find_path_a_star(start, end)"
        )
        ttk.Label(api_frame, text=api_text, justify=tk.LEFT).pack(anchor=tk.W)

    def _bind_events(self) -> None:
        self.canvas.bind("<Button-1>", self._on_left_click)
        self.canvas.bind("<Button-3>", self._on_right_click)
        self.canvas.bind("<Configure>", lambda _event: self.redraw())

    def run(self) -> None:
        self.root.mainloop()

    def redraw(self) -> None:
        self.canvas.delete("all")
        self.canvas_items.clear()

        if not self.model.cells:
            return

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
            fill = self.resolve_cell_color(coord, cell)
            outline = "#ffffff" if coord == self.selected_coord else "#31404d"
            width = 3 if coord == self.selected_coord else 1

            polygon_id = self.canvas.create_polygon(
                points,
                fill=fill,
                outline=outline,
                width=width,
            )
            self.canvas_items[polygon_id] = coord

            label = f"{coord.q},{coord.r}\nh:{cell.height}"
            if not cell.walkable:
                label += "\nX"
            text_color = "#101418" if cell.walkable else "#f5f7fa"
            self.canvas.create_text(
                center_x,
                center_y,
                text=label,
                fill=text_color,
                font=("Consolas", 10, "bold"),
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

    def resolve_cell_color(self, coord: AxialCoord, cell: HexCell) -> str:
        if coord == self.end_coord:
            return "#8b5cf6"
        if coord == self.start_coord:
            return "#3b82f6"
        if cell.color_override:
            return cell.color_override
        if not cell.walkable:
            return "#5b6670"

        normalized = max(-3, min(8, cell.height)) + 3
        ratio = normalized / 11.0

        low = (102, 153, 84)
        high = (206, 167, 94)
        red = int(low[0] + (high[0] - low[0]) * ratio)
        green = int(low[1] + (high[1] - low[1]) * ratio)
        blue = int(low[2] + (high[2] - low[2]) * ratio)
        return f"#{red:02x}{green:02x}{blue:02x}"

    def _get_coord_from_event(self, event: tk.Event) -> Optional[AxialCoord]:
        item_id = self.canvas.find_closest(event.x, event.y)
        if not item_id:
            return None

        coord = self.canvas_items.get(item_id[0])
        if coord is None:
            overlapping = self.canvas.find_overlapping(event.x, event.y, event.x, event.y)
            for candidate in reversed(overlapping):
                coord = self.canvas_items.get(candidate)
                if coord is not None:
                    break

        return coord

    def _on_left_click(self, event: tk.Event) -> None:
        coord = self._get_coord_from_event(event)
        if coord is None:
            return
        self.start_coord = coord
        self._refresh_endpoint_labels()
        self._refresh_path_highlight()
        self.select_cell(coord)

    def _on_right_click(self, event: tk.Event) -> None:
        coord = self._get_coord_from_event(event)
        if coord is None:
            return
        self.end_coord = coord
        self._refresh_endpoint_labels()
        self._refresh_path_highlight()
        self.select_cell(coord)

    def _refresh_endpoint_labels(self) -> None:
        self.start_text.set(self._format_endpoint_text("Start", self.start_coord))
        self.end_text.set(self._format_endpoint_text("End", self.end_coord))

    @staticmethod
    def _format_endpoint_text(label: str, coord: Optional[AxialCoord]) -> str:
        if coord is None:
            return f"{label}: 未设置"
        return f"{label}: Q={coord.q}, R={coord.r}"

    def _refresh_path_highlight(self) -> None:
        self.model.clear_all_colors()

        if self.start_coord is None or self.end_coord is None:
            self.path_text.set("Path: 未设置起点和终点")
            return

        path = self._resolve_active_path()
        algorithm_name = self.algorithm_var.get()
        height_name = "Height On" if self.use_height_rule_var.get() else "Height Off"
        if path:
            self.model.color_cells(path, "#f7c948")
            self.path_text.set(f"Path: {algorithm_name} | {height_name} | {len(path)} 个格子")
            return

        self.path_text.set(f"Path: {algorithm_name} | {height_name} | 未找到可通行路径")

    def _resolve_active_path(self) -> List[AxialCoord]:
        if self.start_coord is None or self.end_coord is None:
            return []

        use_height_rule = self.use_height_rule_var.get()
        algorithm = self.algorithm_var.get()

        if algorithm == "BFS":
            if use_height_rule:
                return self.model.find_path_bfs(self.start_coord, self.end_coord)
            return self.model.find_path_without_height(self.start_coord, self.end_coord)

        if use_height_rule:
            return self.model.find_path_a_star(self.start_coord, self.end_coord)
        return self.model.find_path_a_star_without_height(self.start_coord, self.end_coord)

    def select_cell(self, coord: AxialCoord) -> None:
        cell = self.model.require_cell(coord)
        self.selected_coord = coord
        self.coord_text.set(f"Q={coord.q}, R={coord.r}, S={coord.s}")
        self.height_var.set(cell.height)
        self.walkable_var.set(cell.walkable)
        self.height_scale.set(cell.height)
        self.height_value_label.config(text=str(cell.height))
        self.redraw()

    def _on_height_scale_changed(self, value: str) -> None:
        height = round(float(value))
        self.height_var.set(height)
        self.height_value_label.config(text=str(height))
        self._apply_selected_properties()

    def _apply_selected_properties(self) -> None:
        if self.selected_coord is None:
            return
        self.model.set_height(self.selected_coord, self.height_var.get())
        self.model.set_walkable(self.selected_coord, self.walkable_var.get())
        self._refresh_path_highlight()
        self.redraw()

    def randomize_heights(self) -> None:
        for cell in self.model.cells.values():
            cell.height = random.randint(-1, 5)
            if cell.walkable:
                cell.color_override = None
        self._refresh_path_highlight()
        self.redraw()

    def clear_colors(self) -> None:
        self.model.clear_all_colors()
        self._refresh_path_highlight()
        self.redraw()

    def color_cells(self, coords: Iterable[AxialCoord | Tuple[int, int]], color: str) -> None:
        self.model.color_cells(coords, color)
        self.redraw()

    def demo_highlight_path(self) -> None:
        if self.start_coord is None:
            self.start_coord = AxialCoord(0, 0)
        if self.end_coord is None:
            self.end_coord = AxialCoord(4, 4)
        self._refresh_endpoint_labels()
        self._refresh_path_highlight()
        self.redraw()


def build_demo_grid(width: int = 8, height: int = 7) -> HexGridModel:
    model = HexGridModel(width=width, height=height)

    for coord, cell in model.cells.items():
        cell.height = (coord.q + coord.r) % 4

    for blocked_coord in (AxialCoord(1, 3), AxialCoord(4, 2), AxialCoord(5, 5)):
        if blocked_coord in model.cells:
            model.cells[blocked_coord].walkable = False

    return model


def main() -> None:
    model = build_demo_grid()
    app = HexGridVisualizer(model=model, hex_size=38.0)

    sample_region = [(1, 1), (2, 1), (3, 1)]
    app.color_cells(sample_region, "#8bd3dd")

    app.run()


if __name__ == "__main__":
    main()
