#!/usr/bin/env python3
"""Generate combined sword and preserved slash-trail attack overlays."""

from __future__ import annotations

import json
import math
from pathlib import Path

from PIL import Image, ImageDraw


FRAME_SIZE = 64
FRAME_COUNT = 5
SHEET_SIZE = (FRAME_SIZE * FRAME_COUNT, FRAME_SIZE)
OUTPUT = Path("graphics/effects/attacks/swordsman")
TRAIL_SOURCE = Path("tools/assets/swordsman_slash_trails")
REPORT = Path("tools/swordsman_slash_validation.json")
PREVIEW_OUTPUT = Path("tools/previews/swordsman_attack")
TRANSPARENT = (0, 255, 0)
TRAIL_COLORS = [
    (59, 38, 48), (212, 122, 37), (242, 184, 75),
    (255, 233, 166), (255, 248, 231),
]
SWORD_COLORS = [
    (49, 34, 42), (113, 65, 39), (225, 164, 48),
    (154, 166, 174), (239, 240, 225),
]
COLORS = [TRANSPARENT, *TRAIL_COLORS, *SWORD_COLORS]
COLOR_INDEX = {color: index for index, color in enumerate(COLORS)}

# Screen-space vectors in Direction enum order. Right-facing variants are not
# generated from these vectors: they are same-frame horizontal mirrors of the
# matching left-facing animation, preserving time while reflecting space.
DIRECTIONS = {
    "down": (0, 1),
    "down_left": (-1, 1),
    "left": (-1, 0),
    "up_left": (-1, -1),
    "up": (0, -1),
}
MIRRORS = {
    "up_right": "up_left",
    "right": "left",
    "down_right": "down_left",
}
ORDER = ["down", "down_left", "left", "up_left", "up",
         "up_right", "right", "down_right"]
SWING_ANGLES = (70, 35, 0, -35, -70)
CENTER = (31.5, 31.5)


def palette_data() -> list[int]:
    values = [component for color in COLORS for component in color]
    return values + [0] * (768 - len(values))


def point_at(direction: tuple[float, float], radius: float) -> tuple[int, int]:
    return (round(CENTER[0] + direction[0] * radius),
            round(CENTER[1] + direction[1] * radius))


def rotate(vector: tuple[int, int], degrees: int) -> tuple[float, float]:
    length = math.hypot(*vector)
    x = vector[0] / length
    y = vector[1] / length
    radians = math.radians(degrees)
    cosine = math.cos(radians)
    sine = math.sin(radians)
    return (x * cosine - y * sine, x * sine + y * cosine)


def render_sword(forward: tuple[int, int], angle: int) -> tuple[Image.Image, dict[str, object]]:
    direction = rotate(forward, angle)
    perpendicular = (-direction[1], direction[0])
    frame = Image.new("RGB", (FRAME_SIZE, FRAME_SIZE), TRANSPARENT)
    draw = ImageDraw.Draw(frame)

    grip_start = point_at(direction, 5)
    grip_end = point_at(direction, 11)
    draw.line((grip_start, grip_end), fill=SWORD_COLORS[0], width=5)
    draw.line((grip_start, grip_end), fill=SWORD_COLORS[1], width=3)
    pommel = point_at(direction, 4)
    draw.rectangle((pommel[0] - 2, pommel[1] - 2, pommel[0] + 2, pommel[1] + 2),
                   fill=SWORD_COLORS[0])
    draw.rectangle((pommel[0] - 1, pommel[1] - 1, pommel[0] + 1, pommel[1] + 1),
                   fill=SWORD_COLORS[2])

    guard_center = point_at(direction, 12)
    guard_a = (round(guard_center[0] + perpendicular[0] * 4),
               round(guard_center[1] + perpendicular[1] * 4))
    guard_b = (round(guard_center[0] - perpendicular[0] * 4),
               round(guard_center[1] - perpendicular[1] * 4))
    draw.line((guard_a, guard_b), fill=SWORD_COLORS[0], width=5)
    draw.line((guard_a, guard_b), fill=SWORD_COLORS[2], width=3)

    blade_start = point_at(direction, 13)
    blade_end = point_at(direction, 26)
    draw.line((blade_start, blade_end), fill=SWORD_COLORS[0], width=7)
    draw.line((blade_start, blade_end), fill=SWORD_COLORS[3], width=5)
    highlight_start = (round(blade_start[0] - perpendicular[0]),
                       round(blade_start[1] - perpendicular[1]))
    highlight_end = (round(blade_end[0] - perpendicular[0]),
                     round(blade_end[1] - perpendicular[1]))
    draw.line((highlight_start, highlight_end), fill=SWORD_COLORS[4], width=2)
    sword_tip = point_at(direction, 28)
    draw.line((blade_end, sword_tip), fill=SWORD_COLORS[0], width=3)
    draw.point(point_at(direction, 27), fill=SWORD_COLORS[4])
    return frame, {
        "sword_start": list(grip_start),
        "sword_end": list(sword_tip),
        "rotation_center": list(CENTER),
        "swing_angle": angle,
    }


def mirror_frame(frame: Image.Image, metadata: dict[str, object]) -> tuple[Image.Image, dict[str, object]]:
    result = frame.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
    mirrored = dict(metadata)
    for field in ("sword_start", "sword_end"):
        x, y = metadata[field]
        mirrored[field] = [FRAME_SIZE - 1 - x, y]
    mirrored["rotation_center"] = list(CENTER)
    return result, mirrored


def load_trail_frames(direction: str) -> list[Image.Image]:
    path = TRAIL_SOURCE / f"swordsman_slash_{direction}.bmp"
    raw = path.read_bytes()
    if int.from_bytes(raw[28:30], "little") != 8:
        raise ValueError(f"trail is not indexed 8bpp: {path}")
    with Image.open(path) as sheet:
        if sheet.size != SHEET_SIZE:
            raise ValueError(f"invalid trail sheet size: {path}: {sheet.size}")
        return [sheet.crop((index * FRAME_SIZE, 0, (index + 1) * FRAME_SIZE, FRAME_SIZE)).convert("RGB")
                for index in range(FRAME_COUNT)]


def effect_bbox(frame: Image.Image, allowed_colors: set[tuple[int, int, int]]) -> list[int]:
    pixels = [(x, y) for y in range(FRAME_SIZE) for x in range(FRAME_SIZE)
              if frame.getpixel((x, y)) in allowed_colors]
    if not pixels:
        raise ValueError("required effect layer is empty")
    xs = [point[0] for point in pixels]
    ys = [point[1] for point in pixels]
    return [min(xs), min(ys), max(xs), max(ys)]


def composite(trail: Image.Image, sword: Image.Image) -> Image.Image:
    # The preserved trail is the luminous foreground edge; placing it last
    # keeps even its sparse preparation/recovery pixels visible beside the sword.
    result = sword.copy()
    mask = Image.new("1", trail.size)
    mask.putdata([pixel != TRANSPARENT for pixel in trail.get_flattened_data()])
    result.paste(trail, mask=mask)
    return result


def indexed(image: Image.Image) -> Image.Image:
    result = Image.new("P", image.size)
    result.putpalette(palette_data())
    try:
        result.putdata([COLOR_INDEX[pixel] for pixel in image.get_flattened_data()])
    except KeyError as error:
        raise ValueError(f"unexpected composite color: {error.args[0]}") from error
    return result


def validate_sheet(path: Path, frames: list[Image.Image], metadata: list[dict[str, object]]) -> dict[str, object]:
    raw = path.read_bytes()
    bit_depth = int.from_bytes(raw[28:30], "little")
    with Image.open(path) as sheet:
        if sheet.mode != "P" or sheet.size != SHEET_SIZE or bit_depth != 8:
            raise ValueError(f"invalid output sheet: {path}: {sheet.mode} {sheet.size} {bit_depth}bpp")
        used = set(sheet.get_flattened_data())
        if not used.issubset(set(range(len(COLORS)))):
            raise ValueError(f"unexpected output palette index: {path}")

    frame_report = []
    for index, (frame, geometry) in enumerate(zip(frames, metadata)):
        trail_bbox = effect_bbox(frame, set(TRAIL_COLORS))
        sword_bbox = effect_bbox(frame, set(SWORD_COLORS))
        composite_bbox = effect_bbox(frame, set(TRAIL_COLORS + SWORD_COLORS))
        frame_report.append({
            "frame": index + 1,
            **geometry,
            "slash_bbox_inclusive": trail_bbox,
            "sword_bbox_inclusive": sword_bbox,
            "composite_bbox_inclusive": composite_bbox,
            "top_y": composite_bbox[1],
            "bottom_y": composite_bbox[3],
        })
    return {
        "bmp_size": list(SHEET_SIZE),
        "bit_depth": bit_depth,
        "used_palette_colors": len(used),
        "frames": frame_report,
    }


def save_preview(direction: str, frames: list[Image.Image], metadata: list[dict[str, object]]) -> None:
    scale = 4
    header_height = 24
    cell_width = FRAME_SIZE * scale
    preview = Image.new("RGB", (cell_width * FRAME_COUNT, FRAME_SIZE * scale + header_height),
                        (24, 24, 28))
    draw = ImageDraw.Draw(preview)
    for index, frame in enumerate(frames):
        enlarged = frame.resize((cell_width, FRAME_SIZE * scale), Image.Resampling.NEAREST)
        x = index * cell_width
        preview.paste(enlarged, (x, header_height))
        phase = " START" if index == 0 else " CENTER" if index == 2 else " END" if index == 4 else ""
        draw.text((x + 4, 5), f"{index + 1}{phase}", fill=(255, 255, 255))
        draw.line((x, 0, x, preview.height - 1), fill=(80, 80, 88))

    for index, color in ((0, (0, 220, 255)), (4, (255, 64, 192))):
        tip_x, tip_y = metadata[index]["sword_end"]
        x = index * cell_width + tip_x * scale + scale // 2
        y = header_height + tip_y * scale + scale // 2
        draw.line((x - 6, y, x + 6, y), fill=color, width=2)
        draw.line((x, y - 6, x, y + 6), fill=color, width=2)

    PREVIEW_OUTPUT.mkdir(parents=True, exist_ok=True)
    preview.save(PREVIEW_OUTPUT / f"swordsman_attack_{direction}_preview.png")


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    swords: dict[str, list[Image.Image]] = {}
    geometry: dict[str, list[dict[str, object]]] = {}
    for name, forward in DIRECTIONS.items():
        rendered = [render_sword(forward, angle) for angle in SWING_ANGLES]
        swords[name] = [item[0] for item in rendered]
        geometry[name] = [item[1] for item in rendered]

    for target, source in MIRRORS.items():
        rendered = [mirror_frame(frame, data)
                    for frame, data in zip(swords[source], geometry[source])]
        swords[target] = [item[0] for item in rendered]
        geometry[target] = [item[1] for item in rendered]

    report = {
        "purpose": "combined preserved slash trail and detached sword overlay",
        "frame_size": [FRAME_SIZE, FRAME_SIZE],
        "frame_count": FRAME_COUNT,
        "ticks_per_frame": 2,
        "palette": ["#%02X%02X%02X" % color for color in COLORS],
        "mirror_rule": "same frame index; horizontal pixels only; time is never reversed",
        "directions": {},
    }
    combined: dict[str, list[Image.Image]] = {}
    for direction in ORDER:
        trails = load_trail_frames(direction)
        frames = [composite(trail, sword) for trail, sword in zip(trails, swords[direction])]
        for frame_index, (trail, frame) in enumerate(zip(trails, frames)):
            for trail_pixel, composite_pixel in zip(
                    trail.get_flattened_data(), frame.get_flattened_data()):
                if trail_pixel != TRANSPARENT and trail_pixel != composite_pixel:
                    raise ValueError(
                        f"preserved trail pixel changed: {direction}, frame {frame_index + 1}")
        combined[direction] = frames
        sheet = Image.new("RGB", SHEET_SIZE, TRANSPARENT)
        for index, frame in enumerate(frames):
            sheet.paste(frame, (index * FRAME_SIZE, 0))
        path = OUTPUT / f"swordsman_slash_{direction}.bmp"
        indexed(sheet).save(path)
        report["directions"][direction] = validate_sheet(path, frames, geometry[direction])
        save_preview(direction, frames, geometry[direction])

    for target, source in MIRRORS.items():
        for index in range(FRAME_COUNT):
            expected_sword = swords[source][index].transpose(Image.Transpose.FLIP_LEFT_RIGHT)
            if swords[target][index].tobytes() != expected_sword.tobytes():
                raise ValueError(f"same-index sword mirror failed: {source} -> {target}, frame {index + 1}")
            expected_combined = combined[source][index].transpose(Image.Transpose.FLIP_LEFT_RIGHT)
            if combined[target][index].tobytes() != expected_combined.tobytes():
                raise ValueError(f"same-index composite mirror failed: {source} -> {target}, frame {index + 1}")
        tips = [frame["sword_end"] for frame in geometry[target]]
        if not tips[0][1] < tips[4][1] or geometry[target][2]["swing_angle"] != 0:
            raise ValueError(f"top-to-bottom time order failed: {target}: {tips}")

    REPORT.write_bytes((json.dumps(report, indent=2) + "\n").encode("utf-8"))
    print("generated 8 combined sword+slash sheets: 5x64x64, indexed 8bpp")
    print("validated RIGHT/UP_RIGHT/DOWN_RIGHT as same-index mirrors with top-to-bottom tips")


if __name__ == "__main__":
    main()
