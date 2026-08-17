"""Reconstruct the production player sheet from the enlarged samurai reference."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from statistics import median

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
REFERENCE_PATH = ROOT / "docs/reference/samurai-palette.png"
PRODUCTION_PATH = ROOT / "graphics/characters/heroes/swordsman/swordsman_8dir_sheet.bmp"
PREVIEW_PATH = ROOT / "graphics/characters/heroes/swordsman/swordsman_8dir.png"
REVIEW_DIR = ROOT / "docs/review"
ENLARGED_PATH = REVIEW_DIR / "player_samurai_16color_8dir_preview.png"
COMPARISON_PATH = REVIEW_DIR / "player_samurai_16color_comparison.png"

FRAME_SIZE = 16
FRAME_COUNT = 8
REFERENCE_SCALE = 5
REFERENCE_ORIGINS = (
    (10, 13), (109, 13), (208, 13), (307, 13),
    (10, 110), (109, 110), (208, 110), (307, 110),
)
DIRECTION_ORDER = (
    "DOWN", "DOWN_LEFT", "LEFT", "UP_LEFT",
    "UP", "UP_RIGHT", "RIGHT", "DOWN_RIGHT",
)

# One shared 4bpp palette. Index 0 is the transparent chroma key consumed by Grit.
PALETTE = (
    (0, 255, 0),
    (0, 0, 0),
    (16, 8, 8),
    (24, 24, 24),
    (40, 40, 40),
    (56, 56, 56),
    (80, 80, 80),
    (112, 112, 112),
    (168, 168, 168),
    (216, 216, 216),
    (56, 8, 32),
    (96, 8, 48),
    (144, 8, 64),
    (80, 24, 24),
    (120, 64, 48),
    (192, 112, 72),
)


@dataclass(frozen=True)
class FrameMetrics:
    width: int
    height: int
    min_x: int
    max_x: int
    min_y: int
    max_y: int
    baseline: int
    center_x: float
    occupied_pixels: int


def _is_reference_background(color: tuple[int, int, int]) -> bool:
    red, green, blue = color
    return red > 170 and green < 110 and blue > 170


def _block_color(reference: Image.Image, origin_x: int, origin_y: int,
                 pixel_x: int, pixel_y: int) -> tuple[int, int, int] | None:
    colors = []
    background_count = 0
    block_x = origin_x + pixel_x * REFERENCE_SCALE
    block_y = origin_y + pixel_y * REFERENCE_SCALE

    for y in range(block_y, block_y + REFERENCE_SCALE):
        for x in range(block_x, block_x + REFERENCE_SCALE):
            color = reference.getpixel((x, y))[:3]
            if _is_reference_background(color):
                background_count += 1
            else:
                colors.append(color)

    if background_count >= 13 or not colors:
        return None

    return tuple(round(median(channel)) for channel in zip(*colors))


def _palette_index(color: tuple[int, int, int]) -> int:
    # The reference is a shaded 5x enlargement.  Exaggerate its local contrast
    # before reducing each block to one GBA pixel, or armor planes collapse into black.
    enhanced = tuple(min(255, round(component * 1.75)) for component in color)
    red, green, blue = enhanced
    brightness = (red * 3 + green * 4 + blue * 2) // 9

    if red > 80 and red > green * 1.45 and green > blue * 0.7:
        return 15 if brightness >= 105 else 14

    if red > 55 and red > green * 1.6 and blue > green * 1.25:
        if brightness >= 105:
            return 12
        return 11 if brightness >= 55 else 10

    if red > 50 and red > green * 1.45 and blue < green * 1.15:
        return 13

    if brightness < 22:
        return 1
    if brightness < 38:
        return 3
    if brightness < 58:
        return 4
    if brightness < 82:
        return 5
    if brightness < 115:
        return 6
    if brightness < 155:
        return 7
    if brightness < 205:
        return 8
    return 9


def _extract_frames(reference: Image.Image) -> list[list[list[int]]]:
    frames = []
    for origin_x, origin_y in REFERENCE_ORIGINS:
        frame = []
        for y in range(FRAME_SIZE):
            row = []
            for x in range(FRAME_SIZE):
                color = _block_color(reference, origin_x, origin_y, x, y)
                row.append(0 if color is None else _palette_index(color))
            frame.append(row)
        frames.append(frame)
    return frames


def _read_production_frames() -> list[list[list[int]]]:
    production = Image.open(PRODUCTION_PATH)
    assert production.mode == "P"
    assert production.size == (FRAME_SIZE, FRAME_SIZE * FRAME_COUNT)
    return [
        [
            [production.getpixel((x, frame_index * FRAME_SIZE + y)) for x in range(FRAME_SIZE)]
            for y in range(FRAME_SIZE)
        ]
        for frame_index in range(FRAME_COUNT)
    ]


def _mirror(frame: list[list[int]]) -> list[list[int]]:
    return [list(reversed(row)) for row in frame]


def _flip_lateral_frames(frames: list[list[list[int]]]) -> list[list[list[int]]]:
    """Horizontally flip each lateral frame in place; do not swap frame indexes."""
    result = [[row.copy() for row in frame] for frame in frames]
    for frame_index in (1, 2, 3, 5, 6, 7):
        result[frame_index] = _mirror(frames[frame_index])
    return result


def _metrics(frame: list[list[int]]) -> FrameMetrics:
    points = [(x, y) for y, row in enumerate(frame) for x, value in enumerate(row) if value]
    assert points, "Empty frame"
    min_x = min(x for x, _ in points)
    max_x = max(x for x, _ in points)
    min_y = min(y for _, y in points)
    max_y = max(y for _, y in points)
    return FrameMetrics(
        width=max_x - min_x + 1,
        height=max_y - min_y + 1,
        min_x=min_x,
        max_x=max_x,
        min_y=min_y,
        max_y=max_y,
        baseline=max_y,
        center_x=sum(x for x, _ in points) / len(points),
        occupied_pixels=len(points),
    )


def _longest_run(values: list[int]) -> int:
    longest = 0
    current = 0
    for value in values:
        if value:
            current += 1
            longest = max(longest, current)
        else:
            current = 0
    return longest


def _down_silhouette_metrics(down: list[list[int]]) -> dict[str, int]:
    helmet_rows = down[1:6]
    shoulder_rows = down[5:9]
    torso_rows = [row[3:12] for row in down[8:12]]
    leg_rows = down[13:16]
    baseline_x = [x for x, value in enumerate(down[15]) if value]
    katana_points = [
        (x, y)
        for y in range(12, 15)
        for x in range(11, 16)
        if down[y][x]
    ]
    helmet_y = [y for y in range(1, 6) if any(down[y])]

    return {
        "helmet_width": max(_longest_run(row) for row in helmet_rows),
        "helmet_height": len(helmet_y),
        "shoulder_width": max(_longest_run(row) for row in shoulder_rows),
        "torso_width": max(_longest_run(row) for row in torso_rows),
        "leg_height": sum(any(row[2:11]) for row in leg_rows),
        "leg_thickness": max(
            max(_longest_run(row[2:8]), _longest_run(row[8:13]))
            for row in leg_rows
        ),
        "foot_spread": max(baseline_x) - min(baseline_x) + 1,
        "katana_length": max(
            max(x for x, _ in katana_points) - min(x for x, _ in katana_points) + 1,
            max(y for _, y in katana_points) - min(y for _, y in katana_points) + 1,
        ),
    }


def _validate(frames: list[list[list[int]]]) -> list[FrameMetrics]:
    assert len(frames) == FRAME_COUNT
    assert len(PALETTE) == 16
    assert all(len(frame) == FRAME_SIZE for frame in frames)
    assert all(len(row) == FRAME_SIZE for frame in frames for row in frame)
    assert max(value for frame in frames for row in frame for value in row) < 16

    metrics = [_metrics(frame) for frame in frames]
    assert {metric.baseline for metric in metrics} == {15}, "All feet must share baseline 15"
    assert max(metric.height for metric in metrics) - min(metric.height for metric in metrics) <= 2
    assert all(metric.width >= 10 for metric in metrics), "A direction became too narrow"
    assert all(metric.occupied_pixels >= 65 for metric in metrics), "Body mass is too small"
    assert all(4.0 <= metric.center_x <= 10.5 for metric in metrics), "Direction center jumps too far"

    down = frames[0]
    silhouette = _down_silhouette_metrics(down)
    lower_runs = [_longest_run(down[y]) for y in range(13, 16)]
    assert silhouette["helmet_width"] >= 4, "Helmet does not read separately from a normal head"
    assert silhouette["shoulder_width"] >= 8, "Shoulders are too narrow"
    assert silhouette["torso_width"] >= 6, "Torso became a vertical stick"
    assert max(lower_runs) >= 2, "Legs became one-pixel sticks"
    return metrics


def _indexed_image(width: int, height: int) -> Image.Image:
    image = Image.new("P", (width, height), 0)
    flat_palette = [component for color in PALETTE for component in color]
    image.putpalette(flat_palette + [0] * (768 - len(flat_palette)))
    return image


def _horizontal_indexed(frames: list[list[list[int]]]) -> Image.Image:
    horizontal = _indexed_image(FRAME_SIZE * FRAME_COUNT, FRAME_SIZE)
    for frame_index, frame in enumerate(frames):
        for y, row in enumerate(frame):
            for x, palette_index in enumerate(row):
                horizontal.putpixel((frame_index * FRAME_SIZE + x, y), palette_index)
    return horizontal


def _rgba_preview(indexed: Image.Image) -> Image.Image:
    preview = indexed.convert("RGBA")
    for y in range(indexed.height):
        for x in range(indexed.width):
            index = indexed.getpixel((x, y))
            red, green, blue = PALETTE[index]
            preview.putpixel((x, y), (red, green, blue, 0 if index == 0 else 255))
    return preview


def _frame_preview(frame: list[list[int]]) -> Image.Image:
    indexed = _indexed_image(FRAME_SIZE, FRAME_SIZE)
    for y, row in enumerate(frame):
        for x, palette_index in enumerate(row):
            indexed.putpixel((x, y), palette_index)
    return _rgba_preview(indexed)


def _write_assets(frames: list[list[list[int]]], before_frames: list[list[list[int]]]) -> None:
    production = _indexed_image(FRAME_SIZE, FRAME_SIZE * FRAME_COUNT)
    horizontal = _horizontal_indexed(frames)

    for frame_index, frame in enumerate(frames):
        for y, row in enumerate(frame):
            for x, palette_index in enumerate(row):
                production.putpixel((x, frame_index * FRAME_SIZE + y), palette_index)

    production.save(PRODUCTION_PATH)
    rgba_preview = _rgba_preview(horizontal)
    rgba_preview.save(PREVIEW_PATH)

    REVIEW_DIR.mkdir(parents=True, exist_ok=True)
    enlarged = rgba_preview.resize((128 * 8, 16 * 8), Image.Resampling.NEAREST)
    enlarged.save(ENLARGED_PATH)

    before_preview = _rgba_preview(_horizontal_indexed(before_frames))
    canvas = Image.new("RGB", (1200, 460), (242, 242, 242))
    draw = ImageDraw.Draw(canvas)
    draw.text((24, 18), "Samurai: per-frame horizontal flip (frame indexes unchanged)", fill=(0, 0, 0))
    draw.text((24, 52), "Before", fill=(0, 0, 0))
    draw.text((24, 226), "After: only lateral frames flipped", fill=(0, 0, 0))
    for y, (label, before, after) in enumerate(zip(DIRECTION_ORDER, before_frames, frames)):
        x = 24 + y * 145
        draw.text((x, 78), label, fill=(0, 0, 0))
        before_frame = _frame_preview(before).resize((128, 128), Image.Resampling.NEAREST)
        after_frame = _frame_preview(after).resize((128, 128), Image.Resampling.NEAREST)
        before_background = Image.new("RGB", before_frame.size, (238, 3, 245))
        after_background = Image.new("RGB", after_frame.size, (238, 3, 245))
        before_background.paste(before_frame, mask=before_frame.getchannel("A"))
        after_background.paste(after_frame, mask=after_frame.getchannel("A"))
        canvas.paste(before_background, (x, 96))
        canvas.paste(after_background, (x, 270))
    canvas.save(COMPARISON_PATH)


def main() -> None:
    assert REFERENCE_PATH.exists(), f"Missing reference: {REFERENCE_PATH}"
    reference = Image.open(REFERENCE_PATH).convert("RGB")
    assert reference.size == (396, 205), f"Unexpected reference size: {reference.size}"
    before_frames = _read_production_frames()
    frames = _flip_lateral_frames(before_frames)
    metrics = _validate(frames)
    _write_assets(frames, before_frames)

    print(f"production={PRODUCTION_PATH}")
    print(f"preview={ENLARGED_PATH}")
    print(f"comparison={COMPARISON_PATH}")
    print(f"palette_entries={len(PALETTE)}")
    for direction, metric in zip(DIRECTION_ORDER, metrics):
        print(
            f"{direction}: bounds={metric.min_x},{metric.min_y},"
            f"{metric.width},{metric.height} baseline={metric.baseline} "
            f"center_x={metric.center_x:.2f} occupied={metric.occupied_pixels}"
        )
    silhouette = _down_silhouette_metrics(frames[0])
    print("DOWN silhouette: " + " ".join(
        f"{name}={value}" for name, value in silhouette.items()
    ))
    assert frames[0] == before_frames[0], "DOWN frame must remain pixel-identical"
    assert frames[4] == before_frames[4], "UP frame must remain pixel-identical"
    for frame_index in (1, 2, 3, 5, 6, 7):
        assert frames[frame_index] == _mirror(before_frames[frame_index])
    assert set(value for frame in frames for row in frame for value in row) == \
           set(value for frame in before_frames for row in frame for value in row)
    print("frame_contract=DOWN/UP unchanged; 1,2,3,5,6,7 horizontally flipped in place")


if __name__ == "__main__":
    main()
