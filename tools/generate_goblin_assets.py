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
    (158, 86, 212),  # commit box purple
]
STATUS_PALETTE = [
    TRANSPARENT,
    (42, 42, 42),    # dark neutral keyline
    (82, 82, 82),    # dark neutral-gray, unlit bulb interior
    (181, 126, 39),  # muted gold bulb outline
    (250, 213, 47),  # bright yellow lit bulb interior and rays
    (255, 246, 204), # cream highlight
    (232, 229, 213), # light hourglass frame
    (126, 126, 126), # minimal glass interior
    (192, 125, 48)   # muted-gold sand
]
GOBLIN_PATH = Path("graphics/characters/enemies/goblin/goblin.bmp")
TELEGRAPH_PATH = Path("graphics/effects/common/enemy_telegraph.bmp")
HOURGLASS_PATH = Path("graphics/effects/common/goblin_recovery_hourglass.bmp")
AWARENESS_PATH = Path("graphics/effects/common/goblin_awareness_icons.bmp")
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


def goblin_frame(direction: tuple[int, int], include_club: bool = True) -> Image.Image:
    image = indexed((16, 16))
    draw = ImageDraw.Draw(image)

    # Compact head and body retain an 8px core while the feet stay on row 15.
    draw.polygon(((5, 4), (10, 4), (11, 5), (11, 7), (10, 8), (5, 8), (4, 7), (4, 5)), fill=1)
    draw.polygon(((5, 5), (10, 5), (10, 7), (9, 7), (5, 7), (5, 5)), fill=3)
    draw.polygon(((4, 5), (3, 5), (3, 7), (5, 7)), fill=1)
    draw.polygon(((4, 6), (3, 6), (4, 7), (5, 7)), fill=2)
    draw.polygon(((11, 5), (12, 5), (12, 7), (10, 7)), fill=1)
    draw.polygon(((11, 6), (12, 6), (11, 7), (10, 7)), fill=2)

    if direction[1] >= 0:
        draw.point((6, 6), fill=1)
        draw.point((9, 6), fill=1)
        draw.point((6, 7), fill=4)
        draw.point((9, 7), fill=4)
    elif direction[0] < 0:
        draw.point((6, 6), fill=1)
        draw.point((6, 7), fill=4)
    elif direction[0] > 0:
        draw.point((9, 6), fill=1)
        draw.point((9, 7), fill=4)

    draw.polygon(((5, 10), (10, 10), (11, 11), (10, 13), (9, 13), (9, 15),
                  (8, 15), (8, 14), (7, 14), (7, 15), (5, 15), (5, 13),
                  (4, 12), (4, 11)), fill=1)
    draw.polygon(((5, 11), (10, 11), (10, 12), (9, 12), (9, 13), (8, 13),
                  (8, 12), (7, 12), (7, 13), (6, 13), (6, 12), (5, 12)), fill=2)
    draw.rectangle((5, 9, 10, 9), fill=5)
    draw.rectangle((6, 13, 9, 13), fill=7)
    draw.point((5, 15), fill=2)
    draw.point((9, 15), fill=2)

    if include_club:
        hand_x = 5 if direction[0] < 0 else 10
        hand_y = 11
        end_x = hand_x + direction[0] * 4
        end_y = hand_y + direction[1] * 4
        draw.line(((hand_x, hand_y), (end_x, end_y)), fill=1)
        draw.line(((hand_x, hand_y), (end_x, end_y)), fill=6)
        draw.point((end_x, end_y), fill=1)
    return image


def generate_goblin() -> None:
    GOBLIN_PATH.parent.mkdir(parents=True, exist_ok=True)
    sheet = indexed((16, 16 * len(DIRECTIONS)))
    for index, direction in enumerate(DIRECTIONS):
        sheet.paste(goblin_frame(direction), (0, index * 16))
    sheet.save(GOBLIN_PATH)


def opaque_bounds(image: Image.Image) -> tuple[int, int, int, int]:
    points = [(x, y) for y in range(image.height) for x in range(image.width) if image.getpixel((x, y))]
    assert points
    return min(x for x, _ in points), min(y for _, y in points), max(x for x, _ in points), max(y for _, y in points)


def validate_goblin_frames() -> None:
    direction_names = ("DOWN", "DOWN_LEFT", "LEFT", "UP_LEFT", "UP", "UP_RIGHT", "RIGHT", "DOWN_RIGHT")
    assert len(DIRECTIONS) == len(direction_names)
    for direction_name, direction in zip(direction_names, DIRECTIONS):
        body_bounds = opaque_bounds(goblin_frame(direction, False))
        total_bounds = opaque_bounds(goblin_frame(direction))
        body_width = body_bounds[2] - body_bounds[0] + 1
        body_height = body_bounds[3] - body_bounds[1] + 1
        total_width = total_bounds[2] - total_bounds[0] + 1
        total_height = total_bounds[3] - total_bounds[1] + 1
        assert body_bounds[3] == 15
        assert total_bounds[3] == 15
        assert total_width <= 13 and total_height <= 13
        scarf_pixels = {(x, y) for y in range(16) for x in range(16)
                        if goblin_frame(direction, False).getpixel((x, y)) == 5}
        assert len(scarf_pixels) >= 3
        assert any((x + 1, y) in scarf_pixels for x, y in scarf_pixels)
        print(f"{direction_name}: body+ears={body_width}x{body_height} {body_bounds}, "
              f"with_club={total_width}x{total_height} {total_bounds}")


def generate_telegraph() -> None:
    TELEGRAPH_PATH.parent.mkdir(parents=True, exist_ok=True)
    image = indexed((8, 8))
    draw = ImageDraw.Draw(image)
    # A compact, high-contrast 7px exclamation mark with a one-pixel gap.
    draw.rectangle((2, 0, 5, 4), fill=1)
    draw.rectangle((3, 0, 4, 3), fill=4)
    draw.rectangle((2, 6, 5, 7), fill=1)
    draw.rectangle((3, 6, 4, 6), fill=5)
    image.save(TELEGRAPH_PATH)


def generate_recovery_hourglass() -> None:
    HOURGLASS_PATH.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("P", (8, 16), 0)
    values = [component for color in STATUS_PALETTE for component in color]
    image.putpalette(values + [0] * (768 - len(values)))
    draw = ImageDraw.Draw(image)

    # Both frames keep the same wide-top/wide-bottom X silhouette and narrow waist.
    for offset_y in (0, 8):
        draw.line(((1, offset_y), (6, offset_y)), fill=1)
        draw.line(((2, offset_y), (5, offset_y)), fill=6)
        draw.line(((1, offset_y + 7), (6, offset_y + 7)), fill=1)
        draw.line(((2, offset_y + 7), (5, offset_y + 7)), fill=6)
        draw.point((1, offset_y + 1), fill=1)
        draw.point((2, offset_y + 2), fill=1)
        draw.point((3, offset_y + 3), fill=1)
        draw.point((6, offset_y + 1), fill=1)
        draw.point((5, offset_y + 2), fill=1)
        draw.point((4, offset_y + 3), fill=1)
        draw.point((3, offset_y + 4), fill=1)
        draw.point((2, offset_y + 5), fill=1)
        draw.point((1, offset_y + 6), fill=1)
        draw.point((4, offset_y + 4), fill=1)
        draw.point((5, offset_y + 5), fill=1)
        draw.point((6, offset_y + 6), fill=1)

    # Frame 0 starts with sand in the upper chamber; frame 1 finishes in the lower chamber.
    draw.line(((2, 1), (5, 1)), fill=8)
    draw.line(((3, 2), (4, 2)), fill=8)
    draw.point((3, 3), fill=8)
    draw.point((4, 5), fill=7)
    draw.point((4, 11), fill=7)
    draw.point((3, 12), fill=8)
    draw.line(((3, 13), (4, 13)), fill=8)
    image.save(HOURGLASS_PATH)


def generate_awareness_icons() -> None:
    AWARENESS_PATH.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("P", (8, 24), 0)
    values = [component for color in STATUS_PALETTE for component in color]
    image.putpalette(values + [0] * (768 - len(values)))
    draw = ImageDraw.Draw(image)

    # Frames 0 and 1 share a gold outline, a dark right-and-bottom keyline, and a compact socket.
    for offset_y, interior_color in ((0, 2), (8, 4)):
        draw.point((3, offset_y), fill=3)
        draw.point((4, offset_y), fill=3)
        draw.point((2, offset_y + 1), fill=3)
        draw.point((5, offset_y + 1), fill=3)
        draw.point((1, offset_y + 2), fill=3)
        draw.point((1, offset_y + 3), fill=3)
        draw.point((2, offset_y + 4), fill=3)
        draw.point((3, offset_y + 5), fill=3)
        draw.point((2, offset_y + 2), fill=interior_color)
        draw.rectangle(((3, offset_y + 1), (4, offset_y + 4)), fill=interior_color)
        draw.point((5, offset_y + 2), fill=interior_color)
        draw.point((2, offset_y + 3), fill=interior_color)
        draw.point((5, offset_y + 3), fill=interior_color)
        draw.point((6, offset_y + 2), fill=1)
        draw.point((6, offset_y + 3), fill=1)
        draw.point((5, offset_y + 4), fill=1)
        draw.point((4, offset_y + 5), fill=1)
        draw.line(((2, offset_y + 6), (5, offset_y + 6)), fill=1)
        draw.line(((3, offset_y + 7), (4, offset_y + 7)), fill=2)

    # Frame 1: only the lit bulb gains four short gold rays and a cream highlight.
    draw.point((0, 9), fill=3)
    draw.point((7, 9), fill=3)
    draw.point((0, 12), fill=3)
    draw.point((7, 12), fill=3)
    draw.point((3, 9), fill=5)

    # Frame 2: a bold question mark with a separate dot for readable silhouette.
    draw.line(((2, 17), (5, 17)), fill=1)
    draw.point((1, 18), fill=1)
    draw.point((6, 18), fill=1)
    draw.point((5, 19), fill=1)
    draw.point((4, 20), fill=1)
    draw.point((4, 21), fill=1)
    draw.point((4, 23), fill=1)
    draw.line(((2, 18), (5, 18)), fill=4)
    draw.point((5, 19), fill=4)
    draw.point((4, 20), fill=4)
    image.save(AWARENESS_PATH)


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


def validate(path: Path, size: tuple[int, int], palette: list[tuple[int, int, int]] = PALETTE) -> None:
    raw = path.read_bytes()
    assert int.from_bytes(raw[28:30], "little") == 8
    with Image.open(path) as image:
        assert image.mode == "P"
        assert image.size == size
        assert set(image.get_flattened_data()).issubset(set(range(len(palette))))


def main() -> None:
    generate_goblin()
    validate_goblin_frames()
    generate_telegraph()
    generate_recovery_hourglass()
    generate_awareness_icons()
    generate_debug_corner(DEBUG_CORNER_PATHS["hurt"], 1, False)
    generate_debug_corner(DEBUG_CORNER_PATHS["hit"], 2, False)
    generate_debug_corner(DEBUG_CORNER_PATHS["push"], 3, True)
    generate_debug_corner(DEBUG_CORNER_PATHS["commit"], 4, False)
    validate(GOBLIN_PATH, (16, 128))
    validate(TELEGRAPH_PATH, (8, 8))
    validate(HOURGLASS_PATH, (8, 16), STATUS_PALETTE)
    validate(AWARENESS_PATH, (8, 24), STATUS_PALETTE)
    for path in DEBUG_CORNER_PATHS.values():
        validate(path, (8, 8), DEBUG_PALETTE)
    print("generated indexed 8bpp goblin, enemy telegraph, recovery hourglass, awareness icons, and collision debug corners")


if __name__ == "__main__":
    main()
