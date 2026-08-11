#!/usr/bin/env python3
"""Generate the Stage 1 4bpp-ready regular background source asset."""

from pathlib import Path

from PIL import Image, ImageDraw


OUTPUT_PATH = Path("graphics/backgrounds/stage1_terrain.bmp")

# Stage 1 is a 20x20, 8px-cell world centered inside a 32x32 regular BG.
MAP_SIZE = 32
STAGE_SIZE = 20
TILE_SIZE = 8
STAGE_OFFSET = (MAP_SIZE - STAGE_SIZE) // 2
ROCK_CELLS = {(7, 6), (8, 6), (7, 7), (8, 7)}

# Kept below the 16-color regular-BG limit.  Index zero is the fallback
# backdrop color; terrain itself starts at index three so it remains opaque.
PALETTE = [
    (24, 96, 56),     # fallback backdrop
    (45, 48, 55),     # UI panel base
    (72, 76, 79),     # UI panel inset
    (38, 74, 25),     # grass shadow
    (69, 112, 35),    # grass base
    (102, 142, 48),   # grass light
    (131, 161, 59),   # grass highlight / tufts
    (83, 53, 29),     # dirt shadow
    (132, 85, 39),    # dirt base
    (176, 119, 54),   # dirt light
    (106, 69, 35),    # dirt speckle
    (48, 46, 36),     # rock outline
    (79, 79, 60),     # rock shadow
    (120, 119, 86),   # rock base
    (166, 159, 111),  # rock highlight
    (84, 111, 45),    # rock moss
]


def indexed_image(size: tuple[int, int]) -> Image.Image:
    image = Image.new("P", size, 0)
    image.putpalette([component for color in PALETTE for component in color] + [0] * (768 - len(PALETTE) * 3))
    return image


def dirt_cells() -> set[tuple[int, int]]:
    rows = {
        4: range(3, 8),
        5: range(2, 11),
        6: range(2, 12),
        7: range(2, 12),
        8: range(3, 13),
        9: range(5, 16),
        10: range(6, 17),
        11: range(7, 18),
        12: range(8, 17),
        13: range(7, 15),
    }
    return {(column, row) for row, columns in rows.items() for column in columns}


DIRT_CELLS = dirt_cells()


def grass_tile(variant: int, boundary: bool) -> Image.Image:
    image = indexed_image((TILE_SIZE, TILE_SIZE))
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 7, 7), fill=3 if boundary else 4)

    patterns = (
        ((1, 1), (6, 2), (3, 5), (7, 6)),
        ((2, 0), (5, 3), (0, 5), (6, 7)),
        ((0, 2), (4, 1), (7, 4), (2, 7)),
    )
    for x, y in patterns[variant % len(patterns)][:2 if boundary else 3]:
        draw.point((x, y), fill=4 if boundary else 3)
        if not boundary and x < 7 and y < 7 and (x + y + variant) % 2 == 0:
            draw.point((x + 1, y), fill=5)

    if variant == 2:
        draw.point((4, 5), fill=6)
        draw.point((3, 6), fill=5)
        draw.point((4, 6), fill=6)
        draw.point((5, 6), fill=5)

    if boundary:
        draw.point((1, 1), fill=5)
        draw.point((6, 6), fill=5)
    return image


def dirt_tile(kind: str, variant: int) -> Image.Image:
    image = indexed_image((TILE_SIZE, TILE_SIZE))
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 7, 7), fill=8)

    speckles = (
        ((1, 1), (6, 2), (3, 4), (0, 6), (5, 7)),
        ((2, 0), (5, 2), (1, 5), (7, 6), (4, 7)),
    )[variant]
    for index, (x, y) in enumerate(speckles):
        draw.point((x, y), fill=7 if index % 2 == 0 else 9)
    draw.point((4, 6), fill=10)

    # These named patterns are the complete, finite set used by Stage 1.  The
    # map therefore reuses a small terrain tileset instead of encoding a unique
    # raster tile for every cell.
    edge_pixels = {
        "top": ((0, 0), (1, 0), (3, 0), (4, 0), (6, 0), (1, 1), (4, 1), (6, 1)),
        "bottom": ((1, 7), (2, 7), (4, 7), (5, 7), (7, 7), (2, 6), (5, 6)),
        "left": ((0, 0), (0, 2), (0, 3), (0, 5), (0, 7), (1, 2), (1, 5)),
        "right": ((7, 1), (7, 3), (7, 4), (7, 6), (7, 7), (6, 3), (6, 6)),
    }
    for side in kind.split("_"):
        if side in edge_pixels:
            for x, y in edge_pixels[side]:
                draw.point((x, y), fill=4 if (x + y) % 2 == 0 else 3)
    return image


def dirt_kind(column: int, row: int) -> str:
    neighbors = {
        "top": (column, row - 1),
        "right": (column + 1, row),
        "bottom": (column, row + 1),
        "left": (column - 1, row),
    }
    missing = [side for side, position in neighbors.items() if position not in DIRT_CELLS and position not in ROCK_CELLS]
    if not missing:
        return "base"
    if "top" in missing and "left" in missing:
        return "top_left"
    if "top" in missing and "right" in missing:
        return "top_right"
    if "bottom" in missing and "left" in missing:
        return "bottom_left"
    if "bottom" in missing and "right" in missing:
        return "bottom_right"
    return missing[0]


def rock_image() -> Image.Image:
    image = indexed_image((16, 16))
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 15, 15), fill=8)
    silhouette = [
        (4, 1), (11, 1), (11, 2), (13, 2), (13, 3), (14, 3),
        (14, 5), (15, 5), (15, 11), (14, 11), (14, 13), (12, 13),
        (12, 14), (3, 14), (3, 13), (1, 13), (1, 11), (0, 11),
        (0, 5), (1, 5), (1, 3), (3, 3), (3, 2), (4, 2),
    ]
    draw.polygon(silhouette, fill=11)
    draw.polygon(((4, 3), (10, 3), (10, 4), (12, 4), (12, 6), (13, 6),
                  (13, 10), (11, 10), (11, 12), (4, 12), (4, 11),
                  (2, 11), (2, 6), (3, 6), (3, 4), (4, 4)), fill=13)
    draw.polygon(((5, 3), (9, 3), (9, 4), (11, 4), (11, 6), (8, 6),
                  (8, 7), (4, 7), (4, 5), (5, 5)), fill=14)
    draw.polygon(((2, 8), (5, 8), (5, 9), (8, 9), (8, 12), (4, 12),
                  (4, 11), (2, 11)), fill=12)
    draw.point((6, 4), fill=15)
    draw.point((7, 4), fill=15)
    draw.point((10, 7), fill=12)
    draw.point((11, 8), fill=12)
    draw.point((9, 11), fill=14)
    return image


def panel_tile(image: Image.Image, column: int, row: int) -> None:
    draw = ImageDraw.Draw(image)
    x = column * TILE_SIZE
    y = row * TILE_SIZE
    draw.rectangle((x, y, x + 7, y + 7), fill=1)


def paste_stage_tile(image: Image.Image, tile: Image.Image, column: int, row: int) -> None:
    image.paste(tile, ((STAGE_OFFSET + column) * TILE_SIZE, (STAGE_OFFSET + row) * TILE_SIZE))


def generate() -> Image.Image:
    image = indexed_image((MAP_SIZE * TILE_SIZE, MAP_SIZE * TILE_SIZE))

    for row in range(MAP_SIZE):
        for column in range(MAP_SIZE):
            if 1 <= column <= 5 or 26 <= column <= 30:
                panel_tile(image, column, row)

    for row in range(STAGE_SIZE):
        for column in range(STAGE_SIZE):
            boundary = column in (0, STAGE_SIZE - 1) or row in (0, STAGE_SIZE - 1)
            if (column, row) in ROCK_CELLS:
                continue
            if (column, row) in DIRT_CELLS:
                kind = dirt_kind(column, row)
                variant = 1 if kind == "base" and (column + row) % 3 == 0 else 0
                paste_stage_tile(image, dirt_tile(kind, variant), column, row)
            else:
                variant = (column * 5 + row * 3) % 11
                variant = 2 if variant == 0 else (1 if variant in (3, 7) else 0)
                paste_stage_tile(image, grass_tile(0 if boundary else variant, boundary), column, row)

    rock = rock_image()
    for rock_row in range(2):
        for rock_column in range(2):
            tile = rock.crop((rock_column * TILE_SIZE, rock_row * TILE_SIZE,
                              (rock_column + 1) * TILE_SIZE, (rock_row + 1) * TILE_SIZE))
            paste_stage_tile(image, tile, 7 + rock_column, 6 + rock_row)
    return image


def validate(image: Image.Image) -> None:
    assert image.mode == "P"
    assert image.size == (MAP_SIZE * TILE_SIZE, MAP_SIZE * TILE_SIZE)
    assert max(image.get_flattened_data()) < len(PALETTE)
    assert ROCK_CELLS == {(7, 6), (8, 6), (7, 7), (8, 7)}
    assert STAGE_OFFSET + 7 == 13
    assert STAGE_OFFSET + 6 == 12


def main() -> None:
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    image = generate()
    validate(image)
    image.save(OUTPUT_PATH)
    print("generated Stage 1 terrain: 256x256 indexed BMP, 32x32 regular BG, 20x20 centered Stage")


if __name__ == "__main__":
    main()
