#!/usr/bin/env python3
"""Generate and validate the swordsman's five-frame, eight-direction sword overlay."""

from __future__ import annotations

import json
import math
from pathlib import Path

from PIL import Image, ImageDraw


FRAME_SIZE = 64
FRAME_COUNT = 5
SHEET_SIZE = (FRAME_SIZE * FRAME_COUNT, FRAME_SIZE)
OUTPUT = Path("graphics/effects/attacks/swordsman")
REPORT = Path("tools/swordsman_slash_validation.json")
TRANSPARENT = (0, 255, 0)
COLORS = [
    TRANSPARENT,
    (49, 34, 42),       # dark outline
    (113, 65, 39),      # grip
    (225, 164, 48),     # guard
    (154, 166, 174),    # blade shadow
    (239, 240, 225),    # blade light
]

# Screen-space forward vectors in Direction enum order.
DIRECTIONS = {
    "down": (0, 1),
    "down_left": (-1, 1),
    "left": (-1, 0),
    "up_left": (-1, -1),
    "up": (0, -1),
    "up_right": (1, -1),
    "right": (1, 0),
    "down_right": (1, 1),
}

# Positive rotation starts on the wielder's right-hand side and crosses the
# forward vector to the opposite side. This makes DOWN travel screen-left to
# screen-right and UP travel screen-right to screen-left.
SWING_ANGLES = (70, 35, 0, -35, -70)
CENTER = (31.5, 31.5)


def point_at(direction: tuple[float, float], radius: float) -> tuple[int, int]:
    return (
        round(CENTER[0] + direction[0] * radius),
        round(CENTER[1] + direction[1] * radius),
    )


def rotate(vector: tuple[int, int], degrees: int) -> tuple[float, float]:
    length = math.hypot(*vector)
    x = vector[0] / length
    y = vector[1] / length
    radians = math.radians(degrees)
    cosine = math.cos(radians)
    sine = math.sin(radians)
    return (x * cosine - y * sine, x * sine + y * cosine)


def render_frame(forward: tuple[int, int], swing_angle: int) -> Image.Image:
    direction = rotate(forward, swing_angle)
    perpendicular = (-direction[1], direction[0])
    frame = Image.new("P", (FRAME_SIZE, FRAME_SIZE), 0)
    frame.putpalette([component for color in COLORS for component in color] +
                     [0] * (768 - len(COLORS) * 3))
    draw = ImageDraw.Draw(frame)

    # Grip and pommel are drawn first, close to the implied hand pivot.
    grip_start = point_at(direction, 5)
    grip_end = point_at(direction, 11)
    draw.line((grip_start, grip_end), fill=1, width=5)
    draw.line((grip_start, grip_end), fill=2, width=3)
    pommel = point_at(direction, 4)
    draw.rectangle((pommel[0] - 2, pommel[1] - 2, pommel[0] + 2, pommel[1] + 2), fill=1)
    draw.rectangle((pommel[0] - 1, pommel[1] - 1, pommel[0] + 1, pommel[1] + 1), fill=3)

    # The guard stays perpendicular to the blade in all eight directions.
    guard_center = point_at(direction, 12)
    guard_a = (round(guard_center[0] + perpendicular[0] * 4),
               round(guard_center[1] + perpendicular[1] * 4))
    guard_b = (round(guard_center[0] - perpendicular[0] * 4),
               round(guard_center[1] - perpendicular[1] * 4))
    draw.line((guard_a, guard_b), fill=1, width=5)
    draw.line((guard_a, guard_b), fill=3, width=3)

    # A short, readable blade; every frame uses identical radii and widths.
    blade_start = point_at(direction, 13)
    blade_end = point_at(direction, 26)
    draw.line((blade_start, blade_end), fill=1, width=7)
    draw.line((blade_start, blade_end), fill=4, width=5)
    highlight_start = (round(blade_start[0] - perpendicular[0]),
                       round(blade_start[1] - perpendicular[1]))
    highlight_end = (round(blade_end[0] - perpendicular[0]),
                     round(blade_end[1] - perpendicular[1]))
    draw.line((highlight_start, highlight_end), fill=5, width=2)
    tip = point_at(direction, 28)
    draw.line((blade_end, tip), fill=1, width=3)
    draw.point(point_at(direction, 27), fill=5)
    return frame


def validate(path: Path) -> list[dict[str, object]]:
    raw = path.read_bytes()
    if int.from_bytes(raw[28:30], "little") != 8:
        raise ValueError(f"not indexed 8bpp: {path}")

    frames = []
    with Image.open(path) as image:
        if image.mode != "P" or image.size != SHEET_SIZE:
            raise ValueError(f"invalid sheet: {path}: {image.mode} {image.size}")
        if not set(image.get_flattened_data()).issubset(set(range(len(COLORS)))):
            raise ValueError(f"unexpected palette index: {path}")

        for index in range(FRAME_COUNT):
            frame = image.crop((index * FRAME_SIZE, 0, (index + 1) * FRAME_SIZE, FRAME_SIZE))
            pixels = [(x, y) for y in range(FRAME_SIZE) for x in range(FRAME_SIZE)
                      if frame.getpixel((x, y))]
            if not pixels:
                raise ValueError(f"empty frame {index}: {path}")
            xs = [point[0] for point in pixels]
            ys = [point[1] for point in pixels]
            bbox = [min(xs), min(ys), max(xs), max(ys)]
            if bbox[0] <= 0 or bbox[1] <= 0 or bbox[2] >= 63 or bbox[3] >= 63:
                raise ValueError(f"clipped frame {index}: {path}: {bbox}")
            frames.append({"frame": index + 1, "bbox_inclusive": bbox,
                           "effect_pixels": len(pixels), "swing_angle": SWING_ANGLES[index]})
    return frames


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    report = {
        "purpose": "detached sword overlay; horizontal forward semicircle",
        "sheet_size": list(SHEET_SIZE),
        "frame_size": [FRAME_SIZE, FRAME_SIZE],
        "frame_count": FRAME_COUNT,
        "ticks_per_frame": 2,
        "palette": ["#%02X%02X%02X" % color for color in COLORS],
        "directions": {},
    }

    for name, forward in DIRECTIONS.items():
        sheet = Image.new("P", SHEET_SIZE, 0)
        sheet.putpalette([component for color in COLORS for component in color] +
                         [0] * (768 - len(COLORS) * 3))
        for index, angle in enumerate(SWING_ANGLES):
            sheet.paste(render_frame(forward, angle), (index * FRAME_SIZE, 0))

        path = OUTPUT / f"swordsman_slash_{name}.bmp"
        sheet.save(path)
        (OUTPUT / f"swordsman_slash_{name}.json").write_bytes(
            b'{\n    "type": "sprite",\n    "width": 64,\n    "height": 64\n}\n')
        report["directions"][name] = validate(path)

    REPORT.write_bytes((json.dumps(report, indent=2) + "\n").encode("utf-8"))
    print("generated 8 sword overlays: 5x 64x64 frames, indexed 8bpp")


if __name__ == "__main__":
    main()
