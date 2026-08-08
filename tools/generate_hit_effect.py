#!/usr/bin/env python3
"""Generate and validate the shared four-frame GBA hit effect."""

from pathlib import Path
from PIL import Image

FRAME_SIZE = 16
FRAME_COUNT = 4
OUTPUT = Path("graphics/effects/common/hit_effect.bmp")
TRANSPARENT = (0, 255, 0)
COLORS = [
    TRANSPARENT,
    (255, 255, 255),
    (255, 246, 156),
    (255, 214, 48),
    (255, 139, 24),
    (213, 72, 16),
]

# Each frame is authored on the same 16x16 canvas around (7.5, 7.5).
FRAMES = [
    [
        "................",
        "................",
        "................",
        ".......4........",
        ".......3........",
        ".....4.2.4......",
        "......212.......",
        "...432111234....",
        "......212.......",
        ".....4.2.4......",
        ".......3........",
        ".......4........",
        "................",
        "................",
        "................",
        "................",
    ],
    [
        ".......5........",
        ".......4........",
        "...5...3...5....",
        "....4..2..4.....",
        ".....42124......",
        ".....32123......",
        "....3211123.....",
        ".54322111122345.",
        "....3211123.....",
        ".....32123......",
        ".....42124......",
        "....4..2..4.....",
        "...5...3...5....",
        ".......4........",
        ".......5........",
        "................",
    ],
    [
        "................",
        ".4.....4.....4..",
        "..4...3...4.....",
        "...4..2..4......",
        "....43234.......",
        ".....212........",
        "....21112.......",
        ".5432111112345..",
        "....21112.......",
        ".....212........",
        "....43234.......",
        "...4..2..4......",
        "..4...3...4.....",
        ".4.....4.....4..",
        "5......5......5.",
        "................",
    ],
    [
        "................",
        "..5.........5...",
        "...4.......4....",
        "................",
        ".....3...3......",
        "................",
        ".54.........45..",
        "................",
        "................",
        ".54.........45..",
        "................",
        ".....3...3......",
        "................",
        "...4.......4....",
        "..5.........5...",
        "................",
    ],
]


def main() -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("P", (FRAME_SIZE * FRAME_COUNT, FRAME_SIZE), 0)
    palette = [component for color in COLORS for component in color]
    image.putpalette(palette + [0] * (768 - len(palette)))

    for frame_index, rows in enumerate(FRAMES):
        assert len(rows) == FRAME_SIZE
        for y, row in enumerate(rows):
            assert len(row) == FRAME_SIZE
            for x, value in enumerate(row):
                if value != ".":
                    image.putpixel((frame_index * FRAME_SIZE + x, y), int(value))

    image.save(OUTPUT)
    raw = OUTPUT.read_bytes()
    assert int.from_bytes(raw[28:30], "little") == 8
    with Image.open(OUTPUT) as check:
        assert check.mode == "P"
        assert check.size == (64, 16)
        assert set(check.get_flattened_data()).issubset(set(range(len(COLORS))))
    print(f"generated {OUTPUT}: 64x16, 4x 16x16 frames, indexed 8bpp")


if __name__ == "__main__":
    main()
