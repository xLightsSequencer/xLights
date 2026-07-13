#!/usr/bin/env python3
"""Generates the Voronoi Cells effect's toolbar icon as XPM data, at the five
sizes xLights' EffectIconCache expects (16/24/32/48/64px).

Pure standard library - no PIL/numpy. Computes a brute-force Voronoi diagram
(nearest-of-N-seeds per pixel; the canvas is small enough that this is
instant) with a fixed set of seed points and a "stained glass" palette, then
draws a 1px dark border wherever two neighboring pixels belong to different
regions. Output is valid XPM text for all 5 sizes, printed to stdout - paste
the array definitions directly into VoronoiCellsPlugin.cpp (this script does
not write into the .cpp itself).

Usage:
    python generate_icons.py > voronoi_icons.xpm.txt
"""

import random

SIZES = [16, 24, 32, 48, 64]

# Deep blue, red, green, yellow, purple, cyan - on a dark background (the
# background is itself just another Voronoi region color, since Voronoi
# cells tile the whole canvas with no uncovered "background").
REGION_COLORS = [
    "#1A3AA8",  # deep blue
    "#D62828",  # red
    "#2A9D3C",  # green
    "#E8C400",  # yellow
    "#6A2C91",  # purple
    "#1FB6C9",  # cyan
]
BORDER_COLOR = "#0A0A0F"  # near-black, thin dark border between regions

SEED_COUNT = 6  # one per region color; spec allows 6-8, 6 keeps a 1:1 color mapping


def make_seeds(seed_count, rng_seed=42):
    """Fixed, reproducible seed layout - same rng_seed as the effect's own
    seeded RNG, spread across the canvas rather than left to pure chance so
    small icon sizes still read clearly as distinct cells."""
    rng = random.Random(rng_seed)
    seeds = []
    for i in range(seed_count):
        # Jittered ring layout: a base angle spaced evenly around the
        # canvas plus randomized radius/offset, so cells are distinct at
        # 16x16 but the layout isn't perfectly symmetric/artificial-looking.
        angle = (i / seed_count) * 2 * 3.14159265 + rng.uniform(-0.3, 0.3)
        radius = rng.uniform(0.28, 0.42)
        cx, cy = 0.5, 0.5
        x = cx + radius * _cos(angle)
        y = cy + radius * _sin(angle)
        x = min(max(x + rng.uniform(-0.05, 0.05), 0.04), 0.96)
        y = min(max(y + rng.uniform(-0.05, 0.05), 0.04), 0.96)
        seeds.append((x, y))
    return seeds


def _cos(a):
    import math
    return math.cos(a)


def _sin(a):
    import math
    return math.sin(a)


def nearest_seed(nx, ny, seeds):
    best_i = 0
    best_d = None
    second_d = None
    for i, (sx, sy) in enumerate(seeds):
        dx = nx - sx
        dy = ny - sy
        d = dx * dx + dy * dy
        if best_d is None or d < best_d:
            second_d = best_d
            best_d = d
            best_i = i
        elif second_d is None or d < second_d:
            second_d = d
    return best_i, best_d, second_d


def render_region_grid(size, seeds):
    """Returns a size x size grid of region indices (nearest-seed index)."""
    grid = [[0] * size for _ in range(size)]
    for py in range(size):
        for px in range(size):
            nx = (px + 0.5) / size
            ny = (py + 0.5) / size
            idx, _, _ = nearest_seed(nx, ny, seeds)
            grid[py][px] = idx
    return grid


def is_border(grid, x, y, size):
    idx = grid[y][x]
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        nx2, ny2 = x + dx, y + dy
        if 0 <= nx2 < size and 0 <= ny2 < size and grid[ny2][nx2] != idx:
            return True
    return False


def build_xpm(size, seeds, var_name):
    grid = render_region_grid(size, seeds)

    # Palette: one char per region color + one for the border.
    chars = "123456789abcdefghijklmnopqrstuvwxyz"
    region_chars = [chars[i] for i in range(len(REGION_COLORS))]
    border_char = chars[len(REGION_COLORS)]

    pixel_rows = []
    for y in range(size):
        row = []
        for x in range(size):
            if is_border(grid, x, y, size):
                row.append(border_char)
            else:
                row.append(region_chars[grid[y][x]])
        pixel_rows.append("".join(row))

    lines = []
    lines.append("/* XPM */")
    lines.append(f'static const char *{var_name}[] = {{')
    ncolors = len(REGION_COLORS) + 1
    lines.append(f'"{size} {size} {ncolors} 1",')
    for ch, color in zip(region_chars, REGION_COLORS):
        lines.append(f'"{ch} c {color}",')
    lines.append(f'"{border_char} c {BORDER_COLOR}",')
    for i, row in enumerate(pixel_rows):
        lines.append(f'"{row}",' if i != len(pixel_rows) - 1 else f'"{row}"')
    lines.append("};")
    return "\n".join(lines)


def main():
    seeds = make_seeds(SEED_COUNT)
    for size in SIZES:
        var_name = f"voronoi_{size}"
        print(build_xpm(size, seeds, var_name))
        print()


if __name__ == "__main__":
    main()
