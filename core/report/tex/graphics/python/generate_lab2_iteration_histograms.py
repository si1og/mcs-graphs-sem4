import argparse
import csv
import re
import subprocess
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


INPUT = "10\n7\n3\n2\n8\n0\n9\n0\n"
ITERATIONS_RE = re.compile(r"Количество итераций:\s*(\d+)")


def collect(cli: Path, runs: int) -> list[tuple[int, int, int]]:
    rows = []
    for run in range(1, runs + 1):
        process = subprocess.run(
            [str(cli)],
            input=INPUT,
            text=True,
            capture_output=True,
            check=True,
        )
        values = [int(value) for value in ITERATIONS_RE.findall(process.stdout)]
        if len(values) != 2:
            raise RuntimeError(
                f"Запуск {run}: ожидалось два счётчика итераций, получено {values}"
            )
        rows.append((run, values[0], values[1]))
    return rows


def write_csv(path: Path, rows: list[tuple[int, int, int]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file)
        writer.writerow(("run", "articulation_points", "dijkstra_negative"))
        writer.writerows(rows)


def read_csv(path: Path) -> list[tuple[int, int, int]]:
    with path.open(encoding="utf-8") as file:
        reader = csv.DictReader(file)
        return [
            (
                int(row["run"]),
                int(row["articulation_points"]),
                int(row["dijkstra_negative"]),
            )
            for row in reader
        ]


def plot(
    line_path: Path,
    histogram_path: Path,
    rows: list[tuple[int, int, int]],
) -> None:
    run_numbers = np.array([row[0] for row in rows])
    articulation = np.array([row[1] for row in rows])
    dijkstra = np.array([row[2] for row in rows])

    line_figure, line_axis = plt.subplots(figsize=(10, 6), constrained_layout=True)

    line_axis.bar(
        run_numbers,
        articulation,
        width=0.9,
        color="#4c4ce6",
        alpha=0.68,
        label="Поиск точек сочленения",
    )
    line_axis.bar(
        run_numbers,
        dijkstra,
        width=0.9,
        color="#f05a5a",
        alpha=0.68,
        label="Модифицированный алгоритм Дейкстры",
    )
    line_axis.set_xlabel("Номер запуска")
    line_axis.set_ylabel("Количество итераций")
    line_axis.set_title("Число итераций в 100 запусках на графах из 10 вершин")
    line_axis.grid(alpha=0.25)
    line_axis.legend()
    line_figure.savefig(line_path, dpi=200)
    plt.close(line_figure)

    histogram_figure, histogram_axis = plt.subplots(
        figsize=(10, 6), constrained_layout=True
    )
    bins = np.arange(
        min(articulation.min(), dijkstra.min()) - 5,
        max(articulation.max(), dijkstra.max()) + 16,
        10,
    )
    histogram_axis.hist(
        articulation,
        bins=bins,
        alpha=0.68,
        color="#4c4ce6",
        edgecolor="white",
        linewidth=0.8,
        label="Поиск точек сочленения",
    )
    histogram_axis.hist(
        dijkstra,
        bins=bins,
        alpha=0.68,
        color="#f05a5a",
        edgecolor="white",
        linewidth=0.8,
        label="Модифицированный алгоритм Дейкстры",
    )
    histogram_axis.set_xlabel("Количество итераций")
    histogram_axis.set_ylabel("Количество запусков")
    histogram_axis.set_title("Распределение числа итераций за 100 запусков")
    histogram_axis.grid(axis="y", alpha=0.25)
    histogram_axis.legend()
    histogram_axis.text(
        0.98,
        0.98,
        f"Среднее число итераций:\n"
        f"точки сочленения — {articulation.mean():.1f}\n"
        f"Дейкстра — {dijkstra.mean():.1f}",
        transform=histogram_axis.transAxes,
        ha="right",
        va="top",
        bbox={"facecolor": "white", "alpha": 0.85, "edgecolor": "gray"},
    )

    histogram_figure.savefig(histogram_path, dpi=200)
    plt.close(histogram_figure)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", type=Path)
    parser.add_argument("--runs", type=int, default=100)
    parser.add_argument("--csv", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--histogram-output", type=Path, required=True)
    parser.add_argument("--reuse-csv", action="store_true")
    args = parser.parse_args()

    if args.reuse_csv:
        rows = read_csv(args.csv)
    else:
        if args.cli is None:
            parser.error("--cli is required unless --reuse-csv is used")
        rows = collect(args.cli.resolve(), args.runs)
        write_csv(args.csv, rows)
    plot(args.output, args.histogram_output, rows)

    articulation = np.array([row[1] for row in rows])
    dijkstra = np.array([row[2] for row in rows])
    print(
        f"Точки сочленения: mean={articulation.mean():.1f}, "
        f"min={articulation.min()}, max={articulation.max()}"
    )
    print(
        f"Дейкстра: mean={dijkstra.mean():.1f}, "
        f"min={dijkstra.min()}, max={dijkstra.max()}"
    )


if __name__ == "__main__":
    main()
