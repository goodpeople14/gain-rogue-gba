#!/usr/bin/env python3
"""Generate and validate the swordsman's five-frame, eight-direction slash effects."""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import deque
from pathlib import Path

from PIL import Image


FRAME_SIZE = 64
FRAME_COUNT = 5
SHEET_SIZE = (FRAME_SIZE * FRAME_COUNT, FRAME_SIZE)
GREEN = (0, 255, 0)
PALETTE = {
    "dark": (59, 38, 48),
    "orange": (212, 122, 37),
    "gold": (242, 184, 75),
    "pale": (255, 233, 166),
    "white": (255, 248, 231),
}
CENTER = (24, 24, 39, 39)


def points(*items: tuple[int, int]) -> list[tuple[int, int]]:
    return list(items)


# Approved slash_down v2 is preserved byte-for-byte at the logical pixel level.
DOWN = [
    {"dark": points((23, 40)), "gold": points((22, 40)), "pale": points((21, 40))},
    {
        "dark": points((17, 45), (18, 44), (19, 43), (20, 43), (21, 42), (22, 42),
                       (23, 42), (24, 42)),
        "orange": points((21, 43), (22, 42), (23, 42)),
        "gold": points((24, 42), (25, 42)),
    },
    {
        "dark": points((18, 48), (19, 47), (20, 46), (21, 46), (22, 45), (23, 44),
                       (24, 44), (25, 44), (26, 44)),
        "orange": points((20, 47), (21, 46), (22, 45), (23, 45), (24, 45)),
        "gold": points((23, 46), (24, 45), (25, 44), (26, 44), (27, 44), (28, 44)),
        "pale": points((27, 44), (28, 44), (29, 43), (30, 43)),
        "white": points((30, 43), (31, 42), (32, 42), (33, 42)),
    },
    {
        "dark": points((16, 48), (17, 48), (18, 47), (19, 47), (20, 46), (21, 46),
                       (22, 45), (23, 45), (24, 44), (25, 44)),
        "orange": points((18, 46), (19, 46), (20, 45), (21, 45), (22, 44), (23, 44),
                         (24, 43), (25, 43), (26, 43), (27, 42)),
        "gold": points((24, 44), (25, 44), (26, 43), (27, 43), (28, 42), (29, 42),
                       (30, 42), (31, 42), (32, 42), (33, 42), (34, 42), (35, 42)),
        "pale": points((28, 43), (29, 43), (30, 42), (31, 42), (32, 42), (33, 42),
                       (34, 42), (35, 42), (36, 43), (37, 43), (38, 43), (39, 44)),
        "white": points((32, 41), (33, 41), (34, 41), (35, 41), (36, 42), (37, 42),
                        (38, 42), (39, 43), (40, 43), (41, 44), (42, 44), (43, 45),
                        (44, 45), (45, 46), (46, 46), (47, 47)),
    },
    {
        "orange": points((19, 47), (20, 47)),
        "gold": points((21, 46), (22, 46), (23, 45), (45, 48)),
        "pale": points((34, 43), (35, 43)),
        "white": points((40, 45)),
    },
]

LEFT = [
    {"dark": points((23, 40)), "gold": points((23, 39)), "pale": points((23, 38))},
    {
        "dark": points((23, 39), (22, 38), (22, 37), (21, 36), (21, 35), (21, 34),
                       (20, 33), (20, 32)),
        "orange": points((22, 35), (21, 34), (21, 33)),
        "gold": points((20, 32), (20, 31)),
    },
    {
        "dark": points((23, 41), (22, 40), (21, 39), (21, 38), (20, 37), (20, 36),
                       (19, 35), (19, 34), (18, 33)),
        "orange": points((22, 39), (21, 38), (21, 37), (20, 36), (20, 35)),
        "gold": points((21, 36), (20, 35), (20, 34), (19, 33), (19, 32), (19, 31)),
        "pale": points((19, 32), (18, 31), (18, 30), (18, 29)),
        "white": points((18, 29), (17, 28), (17, 27), (17, 26)),
    },
    {
        "dark": points((23, 47), (23, 46), (22, 45), (22, 44), (21, 43), (21, 42),
                       (20, 41), (20, 40), (19, 39), (19, 38)),
        "orange": points((22, 45), (21, 44), (21, 43), (20, 42), (20, 41), (19, 40),
                         (19, 39), (18, 38), (18, 37), (17, 36)),
        "gold": points((20, 39), (20, 38), (19, 37), (19, 36), (18, 35), (18, 34),
                       (18, 33), (18, 32), (18, 31), (18, 30), (18, 29), (18, 28)),
        "pale": points((19, 35), (19, 34), (18, 33), (18, 32), (18, 31), (18, 30),
                       (18, 29), (18, 28), (19, 27), (19, 26), (19, 25), (20, 24)),
        "white": points((17, 31), (17, 30), (17, 29), (17, 28), (18, 27), (18, 26),
                        (18, 25), (19, 24), (19, 23), (20, 22), (20, 21), (21, 20),
                        (21, 19), (22, 18), (22, 17), (23, 16)),
    },
    {
        "orange": points((22, 44), (22, 43)),
        "gold": points((21, 42), (21, 41), (20, 40), (23, 18)),
        "pale": points((19, 29), (19, 28)),
        "white": points((18, 23)),
    },
]

UP = [
    {"dark": points((23, 23)), "gold": points((22, 23)), "pale": points((21, 23))},
    {
        "dark": points((17, 18), (18, 19), (19, 20), (20, 20), (21, 20), (22, 20),
                       (23, 21), (24, 21)),
        "orange": points((21, 20), (22, 21), (23, 21)),
        "gold": points((24, 21), (25, 21)),
    },
    {
        "dark": points((18, 15), (19, 16), (20, 17), (21, 17), (22, 17), (23, 18),
                       (24, 18), (25, 19), (26, 19)),
        "orange": points((20, 16), (21, 17), (22, 17), (23, 18), (24, 18)),
        "gold": points((23, 17), (24, 18), (25, 18), (26, 19), (27, 19), (28, 19)),
        "pale": points((27, 19), (28, 19), (29, 20), (30, 20)),
        "white": points((30, 20), (31, 21), (32, 21), (33, 21)),
    },
    {
        "dark": points((16, 15), (17, 15), (18, 16), (19, 16), (20, 17), (21, 17),
                       (22, 18), (23, 18), (24, 19), (25, 19)),
        "orange": points((18, 17), (19, 17), (20, 18), (21, 18), (22, 19), (23, 19),
                         (24, 20), (25, 20), (26, 20), (27, 21)),
        "gold": points((24, 19), (25, 19), (26, 20), (27, 20), (28, 21), (29, 21),
                       (30, 21), (31, 21), (32, 21), (33, 21), (34, 21), (35, 21)),
        "pale": points((28, 20), (29, 20), (30, 21), (31, 21), (32, 21), (33, 21),
                       (34, 21), (35, 21), (36, 20), (37, 20), (38, 20), (39, 19)),
        "white": points((32, 22), (33, 22), (34, 22), (35, 22), (36, 21), (37, 21),
                        (38, 21), (39, 20), (40, 20), (41, 19), (42, 19), (43, 18),
                        (44, 18), (45, 17), (46, 17), (47, 16)),
    },
    {
        "orange": points((19, 16), (20, 16)),
        "gold": points((21, 17), (22, 17), (23, 18), (45, 15)),
        "pale": points((34, 20), (35, 20)),
        "white": points((40, 18)),
    },
]

# Diagonal curves are authored against their three-rectangle unions, not rotated.
DOWN_LEFT = [
    {"dark": points((23, 39)), "gold": points((22, 40)), "pale": points((21, 40))},
    {
        "dark": points((23, 39), (22, 40), (21, 41), (20, 42), (19, 43), (18, 44)),
        "orange": points((21, 40), (20, 41), (19, 42)),
        "gold": points((18, 43), (17, 44)),
    },
    {
        "dark": points((23, 35), (22, 36), (21, 37), (20, 38), (19, 39), (18, 40),
                       (17, 41), (16, 42)),
        "orange": points((21, 37), (20, 38), (19, 39), (18, 40)),
        "gold": points((19, 40), (18, 41), (17, 42), (16, 43), (15, 44)),
        "pale": points((17, 43), (16, 44), (15, 45)),
        "white": points((15, 46), (14, 47), (13, 48)),
    },
    {
        "dark": points((23, 32), (22, 33), (21, 34), (20, 35), (19, 36), (18, 37),
                       (17, 38), (16, 39), (15, 40), (14, 41), (13, 42), (12, 43),
                       (11, 44), (10, 45), (9, 46), (8, 47)),
        "orange": points((22, 35), (21, 36), (20, 37), (19, 38), (18, 39), (17, 40),
                         (16, 41), (15, 42), (14, 43), (13, 44)),
        "gold": points((20, 38), (19, 39), (18, 40), (17, 41), (16, 42), (15, 43),
                       (14, 44), (13, 45), (12, 46)),
        "pale": points((18, 41), (17, 42), (16, 43), (15, 44), (14, 45), (13, 46),
                       (12, 47), (11, 48)),
        "white": points((16, 44), (15, 45), (14, 46), (13, 47), (12, 48), (11, 49),
                        (10, 50), (9, 51), (8, 52)),
    },
    {
        "orange": points((21, 36), (20, 37)),
        "gold": points((18, 40), (17, 41), (10, 50)),
        "pale": points((14, 45), (13, 46)),
        "white": points(),
    },
]

UP_LEFT = [
    {"dark": points((23, 24)), "gold": points((22, 23)), "pale": points((21, 23))},
    {
        "dark": points((23, 24), (22, 23), (21, 22), (20, 21), (19, 20), (18, 19)),
        "orange": points((21, 23), (20, 22), (19, 21)),
        "gold": points((18, 20), (17, 19)),
    },
    {
        "dark": points((23, 28), (22, 27), (21, 26), (20, 25), (19, 24), (18, 23),
                       (17, 22), (16, 21)),
        "orange": points((21, 26), (20, 25), (19, 24), (18, 23)),
        "gold": points((19, 23), (18, 22), (17, 21), (16, 20), (15, 19)),
        "pale": points((17, 20), (16, 19), (15, 18)),
        "white": points((15, 17), (14, 16), (13, 15)),
    },
    {
        "dark": points((23, 31), (22, 30), (21, 29), (20, 28), (19, 27), (18, 26),
                       (17, 25), (16, 24), (15, 23), (14, 22), (13, 21), (12, 20),
                       (11, 19), (10, 18), (9, 17), (8, 16)),
        "orange": points((22, 28), (21, 27), (20, 26), (19, 25), (18, 24), (17, 23),
                         (16, 22), (15, 21), (14, 20), (13, 19)),
        "gold": points((20, 25), (19, 24), (18, 23), (17, 22), (16, 21), (15, 20),
                       (14, 19), (13, 18), (12, 17)),
        "pale": points((18, 22), (17, 21), (16, 20), (15, 19), (14, 18), (13, 17),
                       (12, 16), (11, 15)),
        "white": points((16, 19), (15, 18), (14, 17), (13, 16), (12, 15), (11, 14),
                        (10, 13), (9, 12), (8, 11)),
    },
    {
        "orange": points((21, 27), (20, 26)),
        "gold": points((18, 23), (17, 22), (10, 13)),
        "pale": points((14, 18), (13, 17)),
        "white": points(),
    },
]

AUTHORED = {
    "down": DOWN,
    "down_left": DOWN_LEFT,
    "left": LEFT,
    "up_left": UP_LEFT,
    "up": UP,
}
MIRRORS = {"up_right": "up_left", "right": "left", "down_right": "down_left"}
ORDER = ["down", "down_left", "left", "up_left", "up", "up_right", "right", "down_right"]


def mirror(frames):
    return [
        {color: [(FRAME_SIZE - 1 - x, y) for x, y in coords] for color, coords in frame.items()}
        for frame in frames
    ]


def allowed(direction: str, x: int, y: int) -> bool:
    if direction == "down":
        return 16 <= x <= 47 and 40 <= y <= 55
    if direction == "up":
        return 16 <= x <= 47 and 8 <= y <= 23
    if direction == "left":
        return 8 <= x <= 23 and 16 <= y <= 47
    if direction == "right":
        return 40 <= x <= 55 and 16 <= y <= 47
    if direction == "down_left":
        return ((8 <= x <= 23 and 40 <= y <= 55) or
                (24 <= x <= 31 and 40 <= y <= 55) or
                (8 <= x <= 23 and 32 <= y <= 39))
    if direction == "up_left":
        return ((8 <= x <= 23 and 8 <= y <= 23) or
                (24 <= x <= 31 and 8 <= y <= 23) or
                (8 <= x <= 23 and 24 <= y <= 31))
    if direction == "down_right":
        return allowed("down_left", 63 - x, y)
    if direction == "up_right":
        return allowed("up_left", 63 - x, y)
    raise ValueError(direction)


def components(coords: set[tuple[int, int]]) -> int:
    remaining = set(coords)
    count = 0
    while remaining:
        count += 1
        queue = deque([remaining.pop()])
        while queue:
            x, y = queue.popleft()
            for dx, dy in ((-1, -1), (0, -1), (1, -1), (-1, 0),
                           (1, 0), (-1, 1), (0, 1), (1, 1)):
                point = (x + dx, y + dy)
                if point in remaining:
                    remaining.remove(point)
                    queue.append(point)
    return count


def render(direction: str, frames_data):
    frames = []
    reports = []
    for frame_number, frame_data in enumerate(frames_data, 1):
        frame = Image.new("RGB", (FRAME_SIZE, FRAME_SIZE), GREEN)
        pixels = frame.load()
        for color_name, coords in frame_data.items():
            for x, y in coords:
                assert allowed(direction, x, y), (direction, frame_number, x, y)
                assert not (CENTER[0] <= x <= CENTER[2] and CENTER[1] <= y <= CENTER[3])
                pixels[x, y] = PALETTE[color_name]
        opaque = {(x, y) for y in range(FRAME_SIZE) for x in range(FRAME_SIZE)
                  if pixels[x, y] != GREEN}
        assert opaque
        x_values = [x for x, _ in opaque]
        y_values = [y for _, y in opaque]
        bbox = [min(x_values), min(y_values), max(x_values), max(y_values)]
        width = bbox[2] - bbox[0] + 1
        height = bbox[3] - bbox[1] + 1
        if direction in ("down", "up"):
            assert width <= 32
        if direction in ("left", "right"):
            assert height <= 32
        component_count = components(opaque)
        independent_particles = max(0, component_count - 1)
        assert independent_particles <= 3
        reports.append({
            "frame": frame_number,
            "bbox_inclusive": bbox,
            "width": width,
            "height": height,
            "effect_pixels": len(opaque),
            "connected_components": component_count,
            "independent_particles": independent_particles,
            "sha256_rgb": hashlib.sha256(frame.tobytes()).hexdigest(),
        })
        frames.append(frame)
    return frames, reports


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=Path("generated/slash_effects"))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    all_data = dict(AUTHORED)
    for target, source in MIRRORS.items():
        all_data[target] = mirror(all_data[source])

    complete_report = {
        "sheet_size": list(SHEET_SIZE),
        "frame_size": [FRAME_SIZE, FRAME_SIZE],
        "frame_count": FRAME_COUNT,
        "palette": ["#%02X%02X%02X" % value for value in PALETTE.values()],
        "directions": {},
    }
    previews = []
    for direction in ORDER:
        frames, report = render(direction, all_data[direction])
        sheet = Image.new("RGB", SHEET_SIZE, GREEN)
        for index, frame in enumerate(frames):
            sheet.paste(frame, (index * FRAME_SIZE, 0))
        stem = f"swordsman_slash_{direction}"
        sheet.save(args.output / f"{stem}.bmp")
        sheet.save(args.output / f"{stem}.png")
        (args.output / f"{stem}.json").write_text(
            '{\n    "type": "sprite",\n    "height": 64\n}\n', encoding="utf-8"
        )
        preview = sheet.resize((SHEET_SIZE[0] * 8, SHEET_SIZE[1] * 8), Image.Resampling.NEAREST)
        preview.save(args.output / f"{stem}_preview_8x.png")
        previews.append(preview)
        complete_report["directions"][direction] = report

    # Exact mirror validation for every pixel, including the green background.
    for target, source in MIRRORS.items():
        target_frames, _ = render(target, all_data[target])
        source_frames, _ = render(source, all_data[source])
        for target_frame, source_frame in zip(target_frames, source_frames):
            assert target_frame.tobytes() == source_frame.transpose(
                Image.Transpose.FLIP_LEFT_RIGHT).tobytes()

    approved_down_hashes = [
        "4acc9d041b7b49c81319c52ad43dcabbb44ab4ff46ef2567969750b5dd5c8cf2",
        "3e0acd4451f1912578675fcded5c8479d427f10d763772e8cdd21dda54c15b47",
        "9304e2c238d33a036c69e9ff183d971b9d1c72f299c2fecf075e0fe4c7f6a921",
        "3dc3313325b0941950ece917ed43fb57048bf271ae3094dd3049c9c418c6dfee",
        "278a044471f5b1efd6376ce6ef975f9054e9011eb867cd42e34a89b7ed00d5ef",
    ]
    actual_down_hashes = [
        frame["sha256_rgb"] for frame in complete_report["directions"]["down"]
    ]
    assert actual_down_hashes == approved_down_hashes

    montage = Image.new("RGB", (SHEET_SIZE[0] * 8, SHEET_SIZE[1] * 8 * len(previews)), GREEN)
    for index, preview in enumerate(previews):
        montage.paste(preview, (0, index * SHEET_SIZE[1] * 8))
    montage.save(args.output / "swordsman_slash_8dir_preview_8x.png")
    (args.output / "swordsman_slash_validation.json").write_text(
        json.dumps(complete_report, indent=2), encoding="utf-8"
    )
    print(json.dumps(complete_report, indent=2))


if __name__ == "__main__":
    main()
