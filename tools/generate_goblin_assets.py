#!/usr/bin/env python3
"""Generate the prototype goblin and reusable enemy telegraph assets."""

from pathlib import Path

from PIL import Image, ImageDraw


TRANSPARENT = (0, 255, 0)
PALETTE = [
    TRANSPARENT,
    (47, 43, 39),    # outline
    (76, 105, 53),   # skin shadow
    (118, 151, 75),  # skin
    (235, 216, 116), # eyes
    (194, 59, 47),   # scarf
    (107, 64, 35),   # club
    (80, 73, 48),    # loincloth
]
DEBUG_PALETTE = [
    TRANSPARENT,
    (42, 203, 210),  # hurtbox cyan
    (220, 55, 48),   # hitbox red
    (246, 210, 45),  # pushbox yellow
]
COMMIT_PALETTE = [
    TRANSPARENT,
    (158, 86, 212),  # commit box purple
]
GOBLIN_PATH = Path("graphics/characters/enemies/goblin/goblin.bmp")
TELEGRAPH_PATH = Path("graphics/effects/common/enemy_telegraph.bmp")
DEBUG_CORNER_PATHS = {
    "hurt": Path("graphics/effects/common/collision_debug_hurt_corner.bmp"),
    "hit": Path("graphics/effects/common/collision_debug_hit_corner.bmp"),
    "push": Path("graphics/effects/common/collision_debug_push_corner.bmp"),
    "commit": Path("graphics/effects/common/collision_debug_commit_corner.bmp"),
}
DIRECTIONS = ((0, 1), (-1, 1), (-1, 0), (-1, -1),
              (0, -1), (1, -1), (1, 0), (1, 1))


def indexed(size: tuple[int, int]) -> Image.Image:
    image = Image.new("P", size, 0)
    values = [component for color in PALETTE for component in color]
    image.putpalette(values + [0] * (768 - len(values)))
    return image


def goblin_frame(direction: tuple[int, int]) -> Image.Image:
    image = indexed((16, 16))
    draw = ImageDraw.Draw(image)

    # Big head, ears, curved body and red scarf keep the silhouette distinct
    # from the player and training dummy at native GBA resolution.
    draw.polygon(((5, 1), (10, 1), (12, 3), (12, 7), (10, 9), (5, 9), (3, 7), (3, 3)), fill=1)
    draw.polygon(((5, 2), (10, 2), (11, 4), (11, 7), (9, 8), (5, 8), (4, 7), (4, 4)), fill=3)
    draw.polygon(((3, 4), (0, 3), (1, 6), (4, 7)), fill=1)
    draw.polygon(((3, 4), (1, 4), (2, 6), (4, 6)), fill=2)
    draw.polygon(((12, 4), (15, 3), (14, 6), (11, 7)), fill=1)
    draw.polygon(((12, 4), (14, 4), (13, 6), (11, 6)), fill=2)
    draw.rectangle((5, 5, 5, 5), fill=1)
    draw.rectangle((9, 5, 9, 5), fill=1)
    draw.rectangle((5, 6, 5, 6), fill=4)
    draw.rectangle((9, 6, 9, 6), fill=4)
    draw.polygon(((5, 8), (10, 8), (12, 12), (10, 14), (4, 14), (3, 11)), fill=1)
    draw.polygon(((5, 9), (10, 9), (11, 12), (9, 13), (5, 13), (4, 11)), fill=2)
    draw.rectangle((4, 8, 11, 9), fill=5)
    draw.rectangle((5, 10, 10, 11), fill=3)
    draw.rectangle((6, 12, 9, 14), fill=7)
    draw.rectangle((4, 14, 6, 15), fill=1)
    draw.rectangle((9, 14, 11, 15), fill=1)
    draw.rectangle((5, 14, 6, 14), fill=2)
    draw.rectangle((9, 14, 10, 14), fill=2)

    # The club shifts by facing direction without changing its readable length.
    hand_x, hand_y = 11, 11
    end_x = hand_x + direction[0] * 4
    end_y = hand_y + direction[1] * 4
    draw.line(((hand_x, hand_y), (end_x, end_y)), fill=1, width=3)
    draw.line(((hand_x, hand_y), (end_x, end_y)), fill=6, width=1)
    return image


def generate_goblin() -> None:
    GOBLIN_PATH.parent.mkdir(parents=True, exist_ok=True)
    sheet = indexed((16, 16 * len(DIRECTIONS)))
    for index, direction in enumerate(DIRECTIONS):
        sheet.paste(goblin_frame(direction), (0, index * 16))
    sheet.save(GOBLIN_PATH)


def generate_telegraph() -> None:
    TELEGRAPH_PATH.parent.mkdir(parents=True, exist_ok=True)
    image = indexed((8, 16))
    draw = ImageDraw.Draw(image)
    draw.rectangle((2, 0, 5, 10), fill=1)
    draw.rectangle((3, 1, 4, 9), fill=4)
    draw.rectangle((2, 12, 5, 15), fill=1)
    draw.rectangle((3, 13, 4, 14), fill=5)
    image.save(TELEGRAPH_PATH)


def generate_debug_corner(path: Path, color_index: int, dotted: bool, palette: list[tuple[int, int, int]] = DEBUG_PALETTE) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("P", (8, 8), 0)
    values = [component for color in palette for component in color]
    image.putpalette(values + [0] * (768 - len(values)))
    draw = ImageDraw.Draw(image)
    if dotted:
        draw.point((0, 0), fill=color_index)
        draw.point((2, 0), fill=color_index)
        draw.point((0, 2), fill=color_index)
    else:
        draw.line(((0, 0), (4, 0)), fill=color_index)
        draw.line(((0, 0), (0, 4)), fill=color_index)
    image.save(path)


def validate(path: Path, size: tuple[int, int]) -> None:
    raw = path.read_bytes()
    assert int.from_bytes(raw[28:30], "little") == 8
    with Image.open(path) as image:
        assert image.mode == "P"
        assert image.size == size
        assert set(image.get_flattened_data()).issubset(set(range(len(PALETTE))))


def main() -> None:
    generate_goblin()
    generate_telegraph()
    generate_debug_corner(DEBUG_CORNER_PATHS["hurt"], 1, False)
    generate_debug_corner(DEBUG_CORNER_PATHS["hit"], 2, False)
    generate_debug_corner(DEBUG_CORNER_PATHS["push"], 3, True)
    generate_debug_corner(DEBUG_CORNER_PATHS["commit"], 1, False, COMMIT_PALETTE)
    validate(GOBLIN_PATH, (16, 128))
    validate(TELEGRAPH_PATH, (8, 16))
    for path in DEBUG_CORNER_PATHS.values():
        validate(path, (8, 8))
    print("generated indexed 8bpp goblin, enemy telegraph, and collision debug corners")


if __name__ == "__main__":
    main()
