#!/usr/bin/env python3
"""Generate the Stage 2 skeleton background from the Stage 1 tile primitives."""

from pathlib import Path

from PIL import ImageDraw

from generate_stage1_terrain import STAGE_OFFSET, TILE_SIZE, generate


OUTPUT_PATH = Path("graphics/backgrounds/stage2_terrain.bmp")


def main() -> None:
    image = generate()
    palette = image.getpalette()
    # A colder, slate-blue arena makes Stage 2 distinct while retaining the
    # same 4bpp terrain contract and side UI panels.
    replacement = {
        3: (24, 47, 64), 4: (44, 79, 91), 5: (67, 109, 118), 6: (105, 142, 137),
        7: (41, 44, 57), 8: (70, 82, 94), 9: (108, 122, 128), 10: (49, 58, 70),
        11: (30, 38, 48), 12: (54, 67, 75), 13: (85, 101, 105), 14: (139, 147, 132),
        15: (77, 103, 93),
    }
    for index, color in replacement.items():
        palette[index * 3:index * 3 + 3] = color
    image.putpalette(palette)

    draw = ImageDraw.Draw(image)
    for cell_x, cell_y in ((3, 4), (15, 4), (3, 13), (15, 13)):
        x = (STAGE_OFFSET + cell_x) * TILE_SIZE
        y = (STAGE_OFFSET + cell_y) * TILE_SIZE
        draw.rectangle((x, y, x + 15, y + 15), fill=11)
        draw.rectangle((x + 2, y + 2, x + 13, y + 13), fill=13)
        draw.line((x + 3, y + 3, x + 12, y + 3), fill=14)

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT_PATH)
    print("generated Stage 2 terrain: 256x256 indexed BMP, 32x32 regular BG")


if __name__ == "__main__":
    main()
