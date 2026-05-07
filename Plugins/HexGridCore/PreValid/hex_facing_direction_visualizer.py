from __future__ import annotations

import math
import tkinter as tk
from dataclasses import dataclass
from tkinter import ttk
from typing import Dict, List, Tuple


@dataclass(frozen=True, slots=True)
class AxialCoord:
    q: int
    r: int

    @property
    def s(self) -> int:
        return -self.q - self.r


DIRECTION_COLORS: Dict[str, str] = {
    "Right": "#ef4444",
    "TopRight": "#f97316",
    "TopLeft": "#eab308",
    "Left": "#22c55e",
    "BottomLeft": "#06b6d4",
    "BottomRight": "#8b5cf6",
}


DIRECTION_LABELS: Dict[str, str] = {
    "Right": "Right",
    "TopRight": "TopRight",
    "TopLeft": "TopLeft",
    "Left": "Left",
    "BottomLeft": "BottomLeft",
    "BottomRight": "BottomRight",
}


DIRECTION_ORDER = (
    "Right",
    "TopRight",
    "TopLeft",
    "Left",
    "BottomLeft",
    "BottomRight",
)

DIRECTION_CUBE_VECTORS = (
    (1, 0, -1),
    (1, -1, 0),
    (0, -1, 1),
    (-1, 0, 1),
    (-1, 1, 0),
    (0, 1, -1),
)


def get_facing_direction(from_coord: AxialCoord, to_coord: AxialCoord) -> str:
    dq = to_coord.q - from_coord.q
    dr = to_coord.r - from_coord.r
    ds = -dq - dr

    if dq == 0 and dr == 0:
        return "Right"

    max_dot = -999999
    best_dir_index = 0
    for index, direction in enumerate(DIRECTION_CUBE_VECTORS):
        dot = dq * direction[0] + dr * direction[1] + ds * direction[2]
        if dot > max_dot:
            max_dot = dot
            best_dir_index = index
        elif dot == max_dot and index == (best_dir_index + 1) % 6:
            best_dir_index = index

    return DIRECTION_ORDER[best_dir_index]


def hex_distance(a: AxialCoord, b: AxialCoord) -> int:
    return max(abs(a.q - b.q), abs(a.r - b.r), abs(a.s - b.s))


class FacingDirectionVisualizer:
    def __init__(self, radius: int = 7, hex_size: float = 34.0) -> None:
        self.origin = AxialCoord(0, 0)
        self.radius = radius
        self.hex_size = hex_size
        self.coords = self._build_hex_region(radius)

        self.root = tk.Tk()
        self.root.title("Hex Facing Direction Visualizer")
        self.root.geometry("1180x820")

        self.main_frame = ttk.Frame(self.root, padding=12)
        self.main_frame.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self.main_frame, bg="#111827", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.side_panel = ttk.Frame(self.main_frame, width=300)
        self.side_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(12, 0))

        self.hover_text = tk.StringVar(value="Hover a cell")
        self._build_side_panel()
        self.canvas.bind("<Configure>", lambda _event: self.redraw())
        self.canvas.bind("<Motion>", self._on_mouse_move)
        self.redraw()

    def _build_side_panel(self) -> None:
        ttk.Label(
            self.side_panel,
            text="Facing From 0,0",
            font=("Segoe UI", 16, "bold"),
        ).pack(anchor=tk.W)
        ttk.Label(
            self.side_panel,
            text="Each cell is colored by the same cube dot-product rule as GetFacingDirection.",
            justify=tk.LEFT,
            wraplength=280,
        ).pack(anchor=tk.W, pady=(6, 16))

        info_frame = ttk.LabelFrame(self.side_panel, text="Hovered Cell", padding=10)
        info_frame.pack(fill=tk.X)
        ttk.Label(info_frame, textvariable=self.hover_text, justify=tk.LEFT).pack(anchor=tk.W)

        legend_frame = ttk.LabelFrame(self.side_panel, text="Legend", padding=10)
        legend_frame.pack(fill=tk.X, pady=(16, 0))
        for direction, color in DIRECTION_COLORS.items():
            row = ttk.Frame(legend_frame)
            row.pack(fill=tk.X, pady=3)
            swatch = tk.Canvas(row, width=22, height=16, highlightthickness=0)
            swatch.create_rectangle(0, 0, 22, 16, fill=color, outline="")
            swatch.pack(side=tk.LEFT)
            ttk.Label(row, text=DIRECTION_LABELS[direction]).pack(side=tk.LEFT, padx=(8, 0))

        rule_frame = ttk.LabelFrame(self.side_panel, text="Rule", padding=10)
        rule_frame.pack(fill=tk.X, pady=(16, 0))
        ttk.Label(
            rule_frame,
            text=(
                "dq = To.Q - From.Q\n"
                "dr = To.R - From.R\n"
                "ds = -dq - dr\n\n"
                "Dot the delta with the 6 cube direction vectors.\n"
                "Pick the largest dot.\n"
                "Ties go to the next counter-clockwise direction."
            ),
            justify=tk.LEFT,
        ).pack(anchor=tk.W)

    def run(self) -> None:
        self.root.mainloop()

    def redraw(self) -> None:
        self.canvas.delete("all")

        centers = [self.axial_to_pixel(coord) for coord in self.coords]
        min_x = min(point[0] for point in centers) - self.hex_size * 1.4
        max_x = max(point[0] for point in centers) + self.hex_size * 1.4
        min_y = min(point[1] for point in centers) - self.hex_size * 1.4
        max_y = max(point[1] for point in centers) + self.hex_size * 1.4

        canvas_width = max(self.canvas.winfo_width(), 400)
        canvas_height = max(self.canvas.winfo_height(), 400)
        offset_x = (canvas_width - (max_x - min_x)) * 0.5 - min_x
        offset_y = (canvas_height - (max_y - min_y)) * 0.5 - min_y

        for coord in self.coords:
            center_x, center_y = self.axial_to_pixel(coord)
            center_x += offset_x
            center_y += offset_y
            points = self.hex_points(center_x, center_y)

            if coord == self.origin:
                fill = "#f8fafc"
                outline = "#020617"
                text_color = "#020617"
                width = 3
            else:
                direction = get_facing_direction(self.origin, coord)
                fill = DIRECTION_COLORS[direction]
                outline = "#1f2937"
                text_color = "#f8fafc"
                width = 1

            self.canvas.create_polygon(points, fill=fill, outline=outline, width=width, tags=("cell",))
            self.canvas.create_text(
                center_x,
                center_y,
                text=f"{coord.q},{coord.r}",
                fill=text_color,
                font=("Consolas", 9, "bold"),
                justify=tk.CENTER,
            )

        self._draw_axis_labels(offset_x, offset_y)

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

    def _draw_axis_labels(self, offset_x: float, offset_y: float) -> None:
        label_coords = {
            "Right": AxialCoord(self.radius, 0),
            "TopRight": AxialCoord(self.radius, -self.radius),
            "TopLeft": AxialCoord(0, -self.radius),
            "Left": AxialCoord(-self.radius, 0),
            "BottomLeft": AxialCoord(-self.radius, self.radius),
            "BottomRight": AxialCoord(0, self.radius),
        }

        for direction, coord in label_coords.items():
            x, y = self.axial_to_pixel(coord)
            self.canvas.create_text(
                x + offset_x,
                y + offset_y,
                text=DIRECTION_LABELS[direction],
                fill="#020617",
                font=("Segoe UI", 10, "bold"),
            )

    def _on_mouse_move(self, event: tk.Event) -> None:
        coord = self.pixel_to_nearest_axial(event.x, event.y)
        if coord not in self.coords:
            self.hover_text.set("Hover a cell")
            return

        if coord == self.origin:
            self.hover_text.set("Q=0, R=0, S=0\nOrigin")
            return

        direction = get_facing_direction(self.origin, coord)
        dq = coord.q - self.origin.q
        dr = coord.r - self.origin.r
        ds = -dq - dr
        self.hover_text.set(
            f"Q={coord.q}, R={coord.r}, S={coord.s}\n"
            f"dq={dq}, dr={dr}, ds={ds}\n"
            f"Direction: {DIRECTION_LABELS[direction]}"
        )

    def pixel_to_nearest_axial(self, canvas_x: float, canvas_y: float) -> AxialCoord:
        best_coord = min(
            self.coords,
            key=lambda coord: self._canvas_distance_squared(coord, canvas_x, canvas_y),
        )
        return best_coord

    def _canvas_distance_squared(self, coord: AxialCoord, canvas_x: float, canvas_y: float) -> float:
        centers = [self.axial_to_pixel(item) for item in self.coords]
        min_x = min(point[0] for point in centers) - self.hex_size * 1.4
        max_x = max(point[0] for point in centers) + self.hex_size * 1.4
        min_y = min(point[1] for point in centers) - self.hex_size * 1.4
        max_y = max(point[1] for point in centers) + self.hex_size * 1.4
        canvas_width = max(self.canvas.winfo_width(), 400)
        canvas_height = max(self.canvas.winfo_height(), 400)
        offset_x = (canvas_width - (max_x - min_x)) * 0.5 - min_x
        offset_y = (canvas_height - (max_y - min_y)) * 0.5 - min_y

        x, y = self.axial_to_pixel(coord)
        dx = x + offset_x - canvas_x
        dy = y + offset_y - canvas_y
        return dx * dx + dy * dy

    @staticmethod
    def _build_hex_region(radius: int) -> List[AxialCoord]:
        coords: List[AxialCoord] = []
        origin = AxialCoord(0, 0)
        for q in range(-radius, radius + 1):
            for r in range(-radius, radius + 1):
                coord = AxialCoord(q, r)
                if hex_distance(origin, coord) <= radius:
                    coords.append(coord)
        return coords


def main() -> None:
    app = FacingDirectionVisualizer(radius=7, hex_size=34.0)
    app.run()


if __name__ == "__main__":
    main()
