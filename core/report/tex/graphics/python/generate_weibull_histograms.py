from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


SAMPLE_SIZE = 1000
RANDOM_SEED = 42
OUTPUT_DIRECTORY = Path(__file__).parent


def shifted_weibull_sample(
    random_generator: np.random.Generator,
    sample_size: int,
    scale: float,
    shape: float,
    shift: float,
) -> np.ndarray:
    uniform_values = random_generator.random(sample_size)
    return shift + scale * (-np.log(1 - uniform_values)) ** (1 / shape)


def save_histogram(
    samples: list[tuple[np.ndarray, str, str]],
    title: str,
    output_name: str,
    bins: int = 35,
) -> None:
    if len(samples) == 3:
        figure = plt.figure(figsize=(12, 10))
        grid = figure.add_gridspec(2, 4)
        axes = [
            figure.add_subplot(grid[0, 0:2]),
            figure.add_subplot(grid[0, 2:4]),
            figure.add_subplot(grid[1, 1:3]),
        ]
    else:
        figure, axis_row = plt.subplots(
            1,
            len(samples),
            figsize=(6 * len(samples), 5.5),
            squeeze=False,
        )
        axes = list(axis_row[0])

    for axis, (values, label, color) in zip(axes, samples):
        axis.hist(
            values,
            bins=bins,
            alpha=0.68,
            color=color,
            label=label,
            edgecolor="white",
            linewidth=0.4,
        )
        axis.set_xlabel("Значение случайной величины y")
        axis.set_ylabel("Количество значений")
        axis.grid(axis="y", alpha=0.25)
        axis.legend(title="Параметры распределения")

    figure.suptitle(title, fontsize=16)
    figure.tight_layout(rect=(0, 0, 1, 0.95))
    figure.savefig(OUTPUT_DIRECTORY / output_name, dpi=200)
    plt.close(figure)


def save_graph_generation_histogram(
    random_generator: np.random.Generator,
) -> None:
    weibull_values = shifted_weibull_sample(
        random_generator,
        SAMPLE_SIZE,
        scale=0.17,
        shape=2.0,
        shift=0.05,
    )
    degree_values = np.rint(weibull_values * 9).astype(int)

    figure, axes = plt.subplots(1, 2, figsize=(12, 5.5))

    axes[0].hist(
        weibull_values,
        bins=35,
        alpha=0.68,
        color="#4c4ce6",
        edgecolor="white",
        linewidth=0.4,
    )
    axes[0].set_xlabel("Значение случайной величины y")
    axes[0].set_ylabel("Количество значений")
    axes[0].set_title("Выборка распределения")
    axes[0].grid(axis="y", alpha=0.25)

    axes[1].hist(
        degree_values,
        bins=np.arange(-0.5, 10.5, 1),
        alpha=0.68,
        color="#f05a5a",
        edgecolor="white",
        linewidth=0.8,
        rwidth=0.86,
    )
    axes[1].set_xticks(np.arange(0, 10))
    axes[1].set_xlabel("Степень d = round(y · 9)")
    axes[1].set_ylabel("Количество вершин")
    axes[1].set_title("Степени вершин")
    axes[1].grid(axis="y", alpha=0.25)

    figure.suptitle(
        "Распределение, используемое при генерации графа\n"
        "a = 0,17; c = 2,0; y₀ = 0,05",
        fontsize=16,
    )
    figure.tight_layout(rect=(0, 0, 1, 0.9))
    figure.savefig(
        OUTPUT_DIRECTORY / "weibull-graph-generation.png",
        dpi=200,
    )
    plt.close(figure)


def main() -> None:
    random_generator = np.random.default_rng(RANDOM_SEED)

    shape_samples = [
        (
            shifted_weibull_sample(random_generator, SAMPLE_SIZE, 1, 0.5, 0),
            "a = 1; c = 0,5; y₀ = 0",
            "#4c4ce6",
        ),
        (
            shifted_weibull_sample(random_generator, SAMPLE_SIZE, 1, 1.5, 0),
            "a = 1; c = 1,5; y₀ = 0",
            "#f05a5a",
        ),
        (
            shifted_weibull_sample(random_generator, SAMPLE_SIZE, 1, 2.5, 0),
            "a = 1; c = 2,5; y₀ = 0",
            "#8b4fd6",
        ),
    ]
    save_histogram(
        shape_samples,
        "Смещённое распределение Вейбулла при различных значениях c",
        "weibull-shape-comparison.png",
        bins=45,
    )

    save_graph_generation_histogram(random_generator)


if __name__ == "__main__":
    main()
