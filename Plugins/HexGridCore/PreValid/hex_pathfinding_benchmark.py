from __future__ import annotations

import heapq
import json
import random
from dataclasses import asdict, dataclass
from pathlib import Path
from time import perf_counter
from typing import Dict, List, Optional, Tuple

from hex_qr_visualizer import AxialCoord, HexGridModel


RESULTS_PATH = Path(__file__).with_name("hex_pathfinding_benchmark_results.json")


@dataclass(slots=True)
class SearchStats:
    algorithm: str
    use_height_rule: bool
    found_path: bool
    path_length: int
    expanded_nodes: int
    discovered_nodes: int
    frontier_pushes: int
    elapsed_ms: float


@dataclass(slots=True)
class BenchmarkCaseResult:
    case_index: int
    width: int
    height: int
    start: Tuple[int, int]
    end: Tuple[int, int]
    walkable_ratio: float
    bfs: SearchStats
    a_star: SearchStats


def build_random_grid(
    width: int,
    height: int,
    rng: random.Random,
    walkable_probability: float,
    min_height: int,
    max_height: int,
) -> HexGridModel:
    model = HexGridModel(width=width, height=height)
    walkable_count = 0
    for cell in model.cells.values():
        cell.height = rng.randint(min_height, max_height)
        cell.walkable = rng.random() < walkable_probability
        if cell.walkable:
            walkable_count += 1

    if walkable_count < 2:
        walkable_cells = list(model.cells.values())[:2]
        for cell in walkable_cells:
            cell.walkable = True

    return model


def choose_start_end(model: HexGridModel, rng: random.Random) -> Tuple[AxialCoord, AxialCoord]:
    walkable_coords = [coord for coord, cell in model.cells.items() if cell.walkable]
    start, end = rng.sample(walkable_coords, 2)
    return start, end


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


def run_bfs(
    model: HexGridModel,
    start: AxialCoord,
    end: AxialCoord,
    use_height_rule: bool,
) -> SearchStats:
    from queue import Queue

    started_at = perf_counter()
    queue: Queue[Tuple[AxialCoord, List[AxialCoord]]] = Queue()
    queue.put((start, [start]))
    visited = {start}
    expanded_nodes = 0
    frontier_pushes = 1

    while not queue.empty():
        current, path = queue.get()
        expanded_nodes += 1

        if current == end:
            elapsed_ms = (perf_counter() - started_at) * 1000.0
            return SearchStats(
                algorithm="BFS",
                use_height_rule=use_height_rule,
                found_path=True,
                path_length=len(path),
                expanded_nodes=expanded_nodes,
                discovered_nodes=len(visited),
                frontier_pushes=frontier_pushes,
                elapsed_ms=elapsed_ms,
            )

        for neighbor in model.get_neighbors(current, walkable_only=True):
            neighbor_coord = neighbor.coord
            if neighbor_coord in visited:
                continue
            if not model.can_step_to(current, neighbor_coord, use_height_rule=use_height_rule):
                continue
            visited.add(neighbor_coord)
            frontier_pushes += 1
            queue.put((neighbor_coord, path + [neighbor_coord]))

    elapsed_ms = (perf_counter() - started_at) * 1000.0
    return SearchStats(
        algorithm="BFS",
        use_height_rule=use_height_rule,
        found_path=False,
        path_length=0,
        expanded_nodes=expanded_nodes,
        discovered_nodes=len(visited),
        frontier_pushes=frontier_pushes,
        elapsed_ms=elapsed_ms,
    )


def run_a_star(
    model: HexGridModel,
    start: AxialCoord,
    end: AxialCoord,
    use_height_rule: bool,
) -> SearchStats:
    started_at = perf_counter()
    open_heap: List[Tuple[int, int, AxialCoord]] = []
    counter = 0
    came_from: Dict[AxialCoord, AxialCoord] = {}
    g_score: Dict[AxialCoord, int] = {start: 0}
    closed_set = set()
    discovered = {start}
    frontier_pushes = 1
    expanded_nodes = 0

    heapq.heappush(open_heap, (model.hex_distance(start, end), counter, start))

    while open_heap:
        _, _, current = heapq.heappop(open_heap)
        if current in closed_set:
            continue

        expanded_nodes += 1
        if current == end:
            path = reconstruct_path(came_from, current)
            elapsed_ms = (perf_counter() - started_at) * 1000.0
            return SearchStats(
                algorithm="A*",
                use_height_rule=use_height_rule,
                found_path=True,
                path_length=len(path),
                expanded_nodes=expanded_nodes,
                discovered_nodes=len(discovered),
                frontier_pushes=frontier_pushes,
                elapsed_ms=elapsed_ms,
            )

        closed_set.add(current)
        for neighbor in model.get_neighbors(current, walkable_only=True):
            neighbor_coord = neighbor.coord
            if neighbor_coord in closed_set:
                continue
            if not model.can_step_to(current, neighbor_coord, use_height_rule=use_height_rule):
                continue

            tentative_g = g_score[current] + 1
            if tentative_g < g_score.get(neighbor_coord, float("inf")):
                came_from[neighbor_coord] = current
                g_score[neighbor_coord] = tentative_g
                discovered.add(neighbor_coord)
                counter += 1
                frontier_pushes += 1
                f_score = tentative_g + model.hex_distance(neighbor_coord, end)
                heapq.heappush(open_heap, (f_score, counter, neighbor_coord))

    elapsed_ms = (perf_counter() - started_at) * 1000.0
    return SearchStats(
        algorithm="A*",
        use_height_rule=use_height_rule,
        found_path=False,
        path_length=0,
        expanded_nodes=expanded_nodes,
        discovered_nodes=len(discovered),
        frontier_pushes=frontier_pushes,
        elapsed_ms=elapsed_ms,
    )


def average(values: List[float]) -> float:
    return sum(values) / len(values) if values else 0.0


def average_int(values: List[int]) -> float:
    return float(sum(values) / len(values)) if values else 0.0


def summarize_cases(cases: List[BenchmarkCaseResult]) -> Dict[str, object]:
    bfs_times = [case.bfs.elapsed_ms for case in cases]
    astar_times = [case.a_star.elapsed_ms for case in cases]
    bfs_expanded = [case.bfs.expanded_nodes for case in cases]
    astar_expanded = [case.a_star.expanded_nodes for case in cases]
    bfs_found = sum(1 for case in cases if case.bfs.found_path)
    astar_found = sum(1 for case in cases if case.a_star.found_path)
    path_match_count = sum(
        1
        for case in cases
        if case.bfs.found_path == case.a_star.found_path
        and case.bfs.path_length == case.a_star.path_length
    )

    return {
        "case_count": len(cases),
        "bfs": {
            "average_elapsed_ms": average(bfs_times),
            "average_expanded_nodes": average_int(bfs_expanded),
            "average_discovered_nodes": average_int([case.bfs.discovered_nodes for case in cases]),
            "path_found_count": bfs_found,
        },
        "a_star": {
            "average_elapsed_ms": average(astar_times),
            "average_expanded_nodes": average_int(astar_expanded),
            "average_discovered_nodes": average_int([case.a_star.discovered_nodes for case in cases]),
            "path_found_count": astar_found,
        },
        "comparison": {
            "average_time_speedup_ratio_bfs_over_astar": (
                average(bfs_times) / average(astar_times) if average(astar_times) > 0.0 else None
            ),
            "average_expanded_ratio_bfs_over_astar": (
                average_int(bfs_expanded) / average_int(astar_expanded)
                if average_int(astar_expanded) > 0.0
                else None
            ),
            "same_path_length_count": path_match_count,
        },
    }


def run_benchmark_suite(
    *,
    seed: int = 20260422,
    case_count: int = 40,
    width: int = 300,
    height: int = 300,
    walkable_probability: float = 0.9,
    min_height: int = 0,
    max_height: int = 4,
    use_height_rule: bool = True,
    max_generation_attempts: int = 100,
) -> Dict[str, object]:
    rng = random.Random(seed)
    cases: List[BenchmarkCaseResult] = []

    for case_index in range(case_count):
        selected_model: Optional[HexGridModel] = None
        selected_start: Optional[AxialCoord] = None
        selected_end: Optional[AxialCoord] = None
        bfs_result: Optional[SearchStats] = None
        a_star_result: Optional[SearchStats] = None

        for _ in range(max_generation_attempts):
            model = build_random_grid(
                width=width,
                height=height,
                rng=rng,
                walkable_probability=walkable_probability,
                min_height=min_height,
                max_height=max_height,
            )
            start, end = choose_start_end(model, rng)
            trial_bfs_result = run_bfs(model, start, end, use_height_rule=use_height_rule)
            if not trial_bfs_result.found_path:
                continue

            selected_model = model
            selected_start = start
            selected_end = end
            bfs_result = trial_bfs_result
            a_star_result = run_a_star(model, start, end, use_height_rule=use_height_rule)
            break

        if selected_model is None or selected_start is None or selected_end is None or bfs_result is None or a_star_result is None:
            raise RuntimeError(
                f"Unable to generate a solvable random map for case {case_index} "
                f"after {max_generation_attempts} attempts."
            )

        walkable_cells = sum(1 for cell in selected_model.cells.values() if cell.walkable)
        cases.append(
            BenchmarkCaseResult(
                case_index=case_index,
                width=width,
                height=height,
                start=(selected_start.q, selected_start.r),
                end=(selected_end.q, selected_end.r),
                walkable_ratio=walkable_cells / len(selected_model.cells),
                bfs=bfs_result,
                a_star=a_star_result,
            )
        )

    summary = summarize_cases(cases)
    return {
        "config": {
            "seed": seed,
            "case_count": case_count,
            "width": width,
            "height": height,
            "walkable_probability": walkable_probability,
            "min_height": min_height,
            "max_height": max_height,
            "use_height_rule": use_height_rule,
            "max_generation_attempts": max_generation_attempts,
            "max_step_height_diff": HexGridModel(width=1, height=1).MAX_STEP_HEIGHT_DIFF,
        },
        "summary": summary,
        "cases": [asdict(case) for case in cases],
    }


def save_results(results: Dict[str, object], output_path: Path = RESULTS_PATH) -> None:
    output_path.write_text(
        json.dumps(results, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def print_summary(results: Dict[str, object]) -> None:
    summary = results["summary"]
    bfs_summary = summary["bfs"]
    astar_summary = summary["a_star"]
    comparison = summary["comparison"]

    print("Benchmark saved to:", RESULTS_PATH)
    print("Cases:", summary["case_count"])
    print(
        "BFS avg:",
        f"{bfs_summary['average_elapsed_ms']:.3f} ms,",
        f"expanded {bfs_summary['average_expanded_nodes']:.1f},",
        f"found {bfs_summary['path_found_count']}",
    )
    print(
        "A* avg:",
        f"{astar_summary['average_elapsed_ms']:.3f} ms,",
        f"expanded {astar_summary['average_expanded_nodes']:.1f},",
        f"found {astar_summary['path_found_count']}",
    )
    print(
        "Ratios:",
        f"time BFS/A* = {comparison['average_time_speedup_ratio_bfs_over_astar']:.3f},",
        f"expanded BFS/A* = {comparison['average_expanded_ratio_bfs_over_astar']:.3f}",
    )


def main() -> None:
    results = run_benchmark_suite()
    save_results(results)
    print_summary(results)


if __name__ == "__main__":
    main()
