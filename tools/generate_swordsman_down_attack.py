#!/usr/bin/env python3
"""Generate the approved five-frame DOWN attack and its visual verification files."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

from PIL import Image, ImageDraw


FRAME_SIZE = 64
FRAME_COUNT = 5
SHEET_SIZE = (FRAME_SIZE * FRAME_COUNT, FRAME_SIZE)

GREEN = (0, 255, 0)
DARK = (47, 35, 38)
WHITE = (246, 244, 232)
SKIN = (242, 184, 75)
BROWN = (91, 49, 23)
BLADE_DARK = (25, 88, 178)
BLADE_BLUE = (45, 165, 245)
BLADE_LIGHT = (205, 235, 255)

SLASH_DARK = (59, 38, 48)
SLASH_ORANGE = (212, 122, 37)
SLASH_GOLD = (242, 184, 75)
SLASH_PALE = (255, 233, 166)

BODY_TOP_LEFT = (24, 24)
ARM_ROOT = (27, 36)
# The five integer vectors all satisfy dx^2 + dy^2 == 17^2.  This makes the
# blade centreline exactly the same length in every frame, without relying on
# rounded trigonometry.
HAND_PIVOTS = ((28, 41), (30, 38), (32, 37), (34, 38), (36, 41))
BLADE_VECTORS = ((-15, 8), (-8, 15), (0, 17), (8, 15), (15, 8))
BLADE_LENGTH = 17

SLASH_START_X = 13
SLASH_END_X = 51


def _indexed(image: Image.Image) -> Image.Image:
    pixels = list(image.get_flattened_data())
    colors = sorted(set(pixels))
    if GREEN in colors:
        colors.remove(GREEN)
    palette_colors = [GREEN, *colors]
    if len(palette_colors) > 256:
        raise ValueError(f"Too many colors for indexed BMP: {len(palette_colors)}")

    color_to_index = {color: index for index, color in enumerate(palette_colors)}
    indexed = Image.new("P", image.size)
    indexed.putdata([color_to_index[color] for color in pixels])

    flat_palette = [channel for color in palette_colors for channel in color]
    flat_palette.extend([0] * (768 - len(flat_palette)))
    indexed.putpalette(flat_palette)
    return indexed


def _erase_original_gun(body: Image.Image) -> Image.Image:
    body = body.copy()
    pixels = body.load()
    for y in range(10):
        for x in range(13, 16):
            pixels[x, y] = GREEN
    return body


def _load_down_body(gunner_sheet: Path) -> Image.Image:
    sheet = Image.open(gunner_sheet).convert("RGB")
    if sheet.size != (16, 128):
        raise ValueError(f"Expected a 16x128 gunner sheet, got {sheet.size}")

    body = sheet.crop((0, 0, 16, 16))
    pixels = body.load()
    for y in range(body.height):
        for x in range(body.width):
            if pixels[x, y] == (0, 0, 0):
                pixels[x, y] = GREEN
    return _erase_original_gun(body)


def _perpendicular_hilt(pivot: tuple[int, int], vector: tuple[int, int]) -> tuple[tuple[int, int], tuple[int, int]]:
    dx, dy = vector
    length = math.hypot(dx, dy)
    px = -dy / length
    py = dx / length
    first = (round(pivot[0] + px * 2), round(pivot[1] + py * 2))
    second = (round(pivot[0] - px * 2), round(pivot[1] - py * 2))
    return first, second


def _draw_attack_frame(body: Image.Image, frame_index: int) -> Image.Image:
    frame = Image.new("RGB", (FRAME_SIZE, FRAME_SIZE), GREEN)
    frame.paste(body, BODY_TOP_LEFT)
    draw = ImageDraw.Draw(frame)

    pivot = HAND_PIVOTS[frame_index]
    vector = BLADE_VECTORS[frame_index]
    tip = (pivot[0] + vector[0], pivot[1] + vector[1])

    # One identical blade rotates around the same attacking hand.
    draw.line((pivot, tip), fill=BLADE_DARK, width=3)
    draw.line((pivot, tip), fill=BLADE_BLUE, width=1)
    highlight_start = (
        pivot[0] + round(vector[0] * 0.25),
        pivot[1] + round(vector[1] * 0.25),
    )
    draw.line((highlight_start, tip), fill=BLADE_LIGHT, width=1)

    hilt_start, hilt_end = _perpendicular_hilt(pivot, vector)
    draw.line((hilt_start, hilt_end), fill=DARK, width=3)
    draw.line((hilt_start, hilt_end), fill=SKIN, width=1)

    # The screen-left shoulder is the fixed origin of the attacking arm.
    # Frames 4 and 5 cross the torso instead of switching to the other hand.
    draw.line((ARM_ROOT, pivot), fill=DARK, width=4)
    draw.line((ARM_ROOT, pivot), fill=WHITE, width=2)

    handle_end = (
        pivot[0] - round(vector[0] / math.hypot(*vector) * 3),
        pivot[1] - round(vector[1] / math.hypot(*vector) * 3),
    )
    draw.line((pivot, handle_end), fill=DARK, width=3)
    draw.line((pivot, handle_end), fill=BROWN, width=1)

    draw.rectangle((pivot[0] - 1, pivot[1] - 1, pivot[0] + 1, pivot[1] + 1), fill=DARK)
    draw.point(pivot, fill=SKIN)
    return frame


def _slash_y(x: int) -> int:
    distance = x - 32
    return round(54 - (distance * distance) / 72)


def _slash_points() -> list[tuple[int, int]]:
    return [(x, _slash_y(x)) for x in range(SLASH_START_X, SLASH_END_X + 1)]


def _draw_slash_arc(draw: ImageDraw.ImageDraw, frame_index: int) -> None:
    # The complete arc is present in every frame.  Only the bright contact
    # point moves with the blade, so the effect never grows from short to long.
    points = _slash_points()
    draw.line(points, fill=SLASH_DARK, width=3)
    draw.line(points, fill=SLASH_ORANGE, width=2)
    draw.line(points, fill=SLASH_GOLD, width=1)

    tip_x = HAND_PIVOTS[frame_index][0] + BLADE_VECTORS[frame_index][0]
    contact_index = min(
        range(len(points)),
        key=lambda index: abs(points[index][0] - tip_x),
    )
    for index in range(max(0, contact_index - 1), min(len(points), contact_index + 2)):
        draw.point(points[index], fill=SLASH_PALE)


def _draw_slash_frame(frame_index: int) -> Image.Image:
    frame = Image.new("RGB", (FRAME_SIZE, FRAME_SIZE), GREEN)
    draw = ImageDraw.Draw(frame)
    _draw_slash_arc(draw, frame_index)
    return frame


def _sheet(frames: list[Image.Image]) -> Image.Image:
    sheet = Image.new("RGB", SHEET_SIZE, GREEN)
    for index, frame in enumerate(frames):
        sheet.paste(frame, (index * FRAME_SIZE, 0))
    return sheet


def _frames_from_sheet(path: Path) -> list[Image.Image]:
    sheet = Image.open(path).convert("RGB")
    if sheet.size != SHEET_SIZE:
        raise ValueError(f"Expected a {SHEET_SIZE} sheet, got {sheet.size}: {path}")
    return [
        sheet.crop((index * FRAME_SIZE, 0, (index + 1) * FRAME_SIZE, FRAME_SIZE))
        for index in range(FRAME_COUNT)
    ]


def _validate(attack_frames: list[Image.Image], slash_frames: list[Image.Image]) -> None:
    if len(attack_frames) != FRAME_COUNT or len(slash_frames) != FRAME_COUNT:
        raise AssertionError("DOWN attack must contain exactly five frames")

    squared_lengths = [dx * dx + dy * dy for dx, dy in BLADE_VECTORS]
    expected_squared_length = BLADE_LENGTH * BLADE_LENGTH
    if squared_lengths != [expected_squared_length] * FRAME_COUNT:
        raise AssertionError(
            f"Blade centreline lengths are not exactly {BLADE_LENGTH}px: {squared_lengths}"
        )

    hand_x = [point[0] for point in HAND_PIVOTS]
    if hand_x != sorted(hand_x) or len(set(hand_x)) != FRAME_COUNT:
        raise AssertionError(f"Attacking hand must move left-to-right: {hand_x}")

    expected_tip_x = [13, 22, 32, 42, 51]
    actual_tip_x = [
        pivot[0] + vector[0]
        for pivot, vector in zip(HAND_PIVOTS, BLADE_VECTORS)
    ]
    if actual_tip_x != expected_tip_x:
        raise AssertionError(f"Unexpected blade-tip path: {actual_tip_x}")

    blade_colors = {BLADE_DARK, BLADE_BLUE, BLADE_LIGHT}
    for index, frame in enumerate(attack_frames):
        pivot = HAND_PIVOTS[index]
        vector = BLADE_VECTORS[index]
        tip = (pivot[0] + vector[0], pivot[1] + vector[1])
        if frame.getpixel(tip) not in blade_colors:
            raise AssertionError(f"Frame {index + 1} blade tip is missing at {tip}")

    slash_masks = []
    for frame in slash_frames:
        slash_masks.append(
            tuple(pixel != GREEN for pixel in frame.get_flattened_data())
        )
    if any(mask != slash_masks[0] for mask in slash_masks[1:]):
        raise AssertionError("Slash coverage must be identical in all five frames")

    for frame in [*attack_frames, *slash_frames]:
        colors = set(frame.get_flattened_data())
        if GREEN not in colors:
            raise AssertionError("Every frame must retain the transparent green color")


def _save_preview(attack_frames: list[Image.Image], slash_frames: list[Image.Image], output_dir: Path) -> None:
    blade_only_sheet = _sheet(attack_frames).resize(
        (SHEET_SIZE[0] * 6, FRAME_SIZE * 6),
        Image.Resampling.NEAREST,
    )
    blade_only_sheet.save(output_dir / "swordsman_attack_down_blade_only_preview.png")

    composite_frames = []
    for attack, slash in zip(attack_frames, slash_frames):
        composite = slash.copy()
        composite_pixels = composite.load()
        attack_pixels = attack.load()
        for y in range(FRAME_SIZE):
            for x in range(FRAME_SIZE):
                if attack_pixels[x, y] != GREEN:
                    composite_pixels[x, y] = attack_pixels[x, y]
        composite_frames.append(composite)

    preview_sheet = _sheet(composite_frames).resize((SHEET_SIZE[0] * 6, FRAME_SIZE * 6), Image.Resampling.NEAREST)
    preview_sheet.save(output_dir / "swordsman_attack_down_preview.png")

    gif_frames = [
        frame.resize((FRAME_SIZE * 6, FRAME_SIZE * 6), Image.Resampling.NEAREST)
        for frame in composite_frames
    ]
    gif_frames[0].save(
        output_dir / "swordsman_attack_down_preview.gif",
        save_all=True,
        append_images=gif_frames[1:],
        duration=[120, 120, 120, 120, 220],
        loop=0,
        disposal=2,
    )

    report = {
        "blade_length_pixels": BLADE_LENGTH,
        "blade_vectors": [list(vector) for vector in BLADE_VECTORS],
        "blade_squared_lengths": [
            vector[0] * vector[0] + vector[1] * vector[1]
            for vector in BLADE_VECTORS
        ],
        "hand_pivots": [list(pivot) for pivot in HAND_PIVOTS],
        "slash_x_range": [SLASH_START_X, SLASH_END_X],
        "slash_coverage_identical": True,
    }
    (output_dir / "swordsman_attack_down_validation.json").write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--preview-dir", type=Path)
    args = parser.parse_args()

    repo_root = args.repo_root.resolve()
    preview_dir = (args.preview_dir or repo_root / "build" / "attack-preview").resolve()
    preview_dir.mkdir(parents=True, exist_ok=True)

    gunner_sheet = repo_root / "graphics/characters/heroes/gunner/gunner_8dir_sheet.bmp"
    output_dir = repo_root / "graphics/effects/attacks/swordsman"
    body = _load_down_body(gunner_sheet)

    attack_frames = [_draw_attack_frame(body, index) for index in range(FRAME_COUNT)]
    slash_frames = [_draw_slash_frame(index) for index in range(FRAME_COUNT)]
    _validate(attack_frames, slash_frames)

    attack_path = output_dir / "swordsman_attack_down.bmp"
    slash_path = output_dir / "swordsman_slash_down.bmp"
    _indexed(_sheet(attack_frames)).save(attack_path)
    _indexed(_sheet(slash_frames)).save(slash_path)

    # Verification files must be derived from the final game BMPs, not the
    # pre-conversion working images.
    final_attack_frames = _frames_from_sheet(attack_path)
    final_slash_frames = _frames_from_sheet(slash_path)
    _validate(final_attack_frames, final_slash_frames)
    _save_preview(final_attack_frames, final_slash_frames, preview_dir)

    print(f"attack={attack_path}")
    print(f"slash={slash_path}")
    print(f"preview={preview_dir / 'swordsman_attack_down_preview.png'}")
    print(f"gif={preview_dir / 'swordsman_attack_down_preview.gif'}")
    print(f"validation={preview_dir / 'swordsman_attack_down_validation.json'}")


if __name__ == "__main__":
    main()
