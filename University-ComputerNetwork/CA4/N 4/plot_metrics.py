#!/usr/bin/env python3

import argparse
import csv
import html
from pathlib import Path


def read_columns(filename, *columns):
    values = {column: [] for column in columns}
    with open(filename, newline="", encoding="utf-8") as csv_file:
        for row in csv.DictReader(csv_file):
            for column in columns:
                values[column].append(float(row[column]))
    return [values[column] for column in columns]


def read_cwnd(filename):
    time_ms = []
    cwnd = []
    ssthresh = []
    fast_retransmits = []
    with open(filename, newline="", encoding="utf-8") as csv_file:
        for row in csv.DictReader(csv_file):
            time_value = float(row["time_ms"])
            cwnd_value = float(row["cwnd_packets"])
            time_ms.append(time_value)
            cwnd.append(cwnd_value)
            ssthresh.append(float(row["ssthresh_packets"]))
            if row.get("event") == "FAST_RETRANSMIT":
                fast_retransmits.append((time_value, cwnd_value))
    return time_ms, cwnd, ssthresh, fast_retransmits


def points(values_x, values_y, left, top, width, height, max_x, max_y):
    coordinates = []
    for value_x, value_y in zip(values_x, values_y):
        x = left + (value_x / max_x) * width if max_x else left
        y = top + height - (value_y / max_y) * height if max_y else top + height
        coordinates.append(f"{x:.2f},{y:.2f}")
    return " ".join(coordinates)


def write_chart(filename, title, x_values, series, x_label, y_label, markers=None):
    canvas_width = 1000
    canvas_height = 560
    left = 85
    top = 55
    plot_width = 870
    plot_height = 420
    max_x = max(x_values, default=1.0) or 1.0
    max_y = max((max(values, default=0.0) for _, values, _ in series), default=1.0) or 1.0

    svg = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{canvas_width}" height="{canvas_height}" viewBox="0 0 {canvas_width} {canvas_height}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{canvas_width / 2}" y="30" text-anchor="middle" font-family="sans-serif" font-size="22">{html.escape(title)}</text>',
    ]

    for tick in range(6):
        ratio = tick / 5
        x = left + ratio * plot_width
        y = top + plot_height - ratio * plot_height
        x_value = ratio * max_x
        y_value = ratio * max_y
        svg.append(f'<line x1="{x:.2f}" y1="{top}" x2="{x:.2f}" y2="{top + plot_height}" stroke="#e5e7eb"/>')
        svg.append(f'<line x1="{left}" y1="{y:.2f}" x2="{left + plot_width}" y2="{y:.2f}" stroke="#e5e7eb"/>')
        svg.append(f'<text x="{x:.2f}" y="{top + plot_height + 22}" text-anchor="middle" font-family="sans-serif" font-size="12">{x_value:.0f}</text>')
        svg.append(f'<text x="{left - 12}" y="{y + 4:.2f}" text-anchor="end" font-family="sans-serif" font-size="12">{y_value:.1f}</text>')

    svg.extend([
        f'<line x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}" stroke="black"/>',
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}" stroke="black"/>',
        f'<text x="{left + plot_width / 2}" y="{canvas_height - 20}" text-anchor="middle" font-family="sans-serif" font-size="15">{html.escape(x_label)}</text>',
        f'<text x="20" y="{top + plot_height / 2}" text-anchor="middle" transform="rotate(-90 20 {top + plot_height / 2})" font-family="sans-serif" font-size="15">{html.escape(y_label)}</text>',
    ])

    for index, (name, values, color) in enumerate(series):
        line_points = points(x_values, values, left, top, plot_width, plot_height, max_x, max_y)
        svg.append(f'<polyline points="{line_points}" fill="none" stroke="{color}" stroke-width="2.5"/>')
        legend_y = 55 + index * 24
        svg.append(f'<line x1="800" y1="{legend_y}" x2="830" y2="{legend_y}" stroke="{color}" stroke-width="3"/>')
        svg.append(f'<text x="838" y="{legend_y + 5}" font-family="sans-serif" font-size="14">{html.escape(name)}</text>')

    if markers:
        for marker_x, marker_y in markers:
            x = left + (marker_x / max_x) * plot_width if max_x else left
            y = top + plot_height - (marker_y / max_y) * plot_height if max_y else top + plot_height
            svg.append(
                f'<circle cx="{x:.2f}" cy="{y:.2f}" r="6" fill="#f59e0b" '
                'stroke="#92400e" stroke-width="2"/>'
            )
        legend_y = 55 + len(series) * 24
        svg.append(
            f'<circle cx="815" cy="{legend_y}" r="6" fill="#f59e0b" '
            'stroke="#92400e" stroke-width="2"/>'
        )
        svg.append(
            f'<text x="838" y="{legend_y + 5}" font-family="sans-serif" '
            'font-size="14">Fast Retransmit</text>'
        )

    svg.append('</svg>')
    with open(filename, "w", encoding="utf-8") as output_file:
        output_file.write("\n".join(svg))


def main():
    parser = argparse.ArgumentParser(description="Create congestion-control metric charts.")
    parser.add_argument("--cwnd", default="cwnd.csv", help="cwnd CSV input")
    parser.add_argument("--throughput", default="throughput.csv", help="throughput CSV input")
    parser.add_argument("--output-dir", default=".", help="chart output directory")
    parser.add_argument("--title-prefix", default="", help="text prepended to chart titles")
    arguments = parser.parse_args()

    output_directory = Path(arguments.output_dir)
    output_directory.mkdir(parents=True, exist_ok=True)
    title_prefix = f"{arguments.title_prefix} - " if arguments.title_prefix else ""

    time_ms, cwnd, ssthresh, fast_retransmits = read_cwnd(arguments.cwnd)
    write_chart(
        output_directory / "cwnd.svg",
        f"{title_prefix}Congestion Window Over Time",
        time_ms,
        [("cwnd", cwnd, "#2563eb"), ("ssthresh", ssthresh, "#dc2626")],
        "Time (ms)",
        "Window (packets)",
        fast_retransmits,
    )

    time_ms, throughput = read_columns(
        arguments.throughput, "time_ms", "throughput_kbps"
    )
    write_chart(
        output_directory / "throughput.svg",
        f"{title_prefix}Throughput Over Time",
        time_ms,
        [("throughput", throughput, "#16a34a")],
        "Time (ms)",
        "Throughput (Kbps)",
    )

    print(f"Created charts in {output_directory}")


if __name__ == "__main__":
    main()
