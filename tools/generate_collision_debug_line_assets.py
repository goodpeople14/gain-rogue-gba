"""Generates fixed-size, non-affine Collision Debug guide segments."""

from collections import Counter
from pathlib import Path

from PIL import Image, ImageDraw


ASSET_DIRECTORY = Path(__file__).resolve().parents[1] / "graphics" / "effects" / "common"
ASSET_NAMES = (
    "collision_debug_commit",
    "collision_debug_hit",
    "collision_debug_hurt",
    "collision_debug_push",
    "collision_debug_static_obstacle",
)
TRANSPARENT_RGB = (0, 255, 0)
LINE_THICKNESS = 1

# These are the valid GBA OBJ shapes used by CollisionDebugOverlay.  The
# visible stroke can be shorter than its transparent OBJ canvas so 6/10/12px
# physical collision boxes remain exact without affine scaling.
HORIZONTAL_SEGMENTS = (
    ("6", 8, 8, 6),
    ("8", 8, 8, 8),
    ("10", 16, 8, 10),
    ("12", 16, 8, 12),
    ("16", 16, 8, 16),
    ("32", 32, 8, 32),
    ("64", 64, 32, 64),
)
VERTICAL_SEGMENTS = (
    ("6", 8, 8, 6),
    ("8", 8, 8, 8),
    ("10", 8, 16, 10),
    ("12", 8, 16, 12),
    ("16", 8, 16, 16),
    ("32", 8, 32, 32),
)


def palette_color(palette: list[int], index: int) -> tuple[int, int, int]:
    offset = index * 3
    return tuple(palette[offset:offset + 3])


def write_sprite_metadata(path: Path, width: int, height: int) -> None:
    path.write_text(
        '{\n'
        '    "type": "sprite",\n'
        f'    "width": {width},\n'
        f'    "height": {height}\n'
        '}\n',
        encoding="utf-8",
    )


def generate_line_assets(asset_name: str) -> None:
    source = Image.open(ASSET_DIRECTORY / f"{asset_name}_corner.bmp")
    if source.mode != "P":
        raise ValueError(f"{source.filename} must be palette-indexed")

    palette = source.getpalette()
    color_counts = Counter(source.get_flattened_data())
    transparent_index = next(
        index for index in color_counts if palette_color(palette, index) == TRANSPARENT_RGB
    )
    stroke_index = next(
        index for index, _ in color_counts.most_common() if index != transparent_index
    )

    for suffix, width, height, visible_length in HORIZONTAL_SEGMENTS:
        horizontal = Image.new("P", (width, height), transparent_index)
        horizontal.putpalette(palette)
        left = (width - visible_length) // 2
        center_y = height // 2
        ImageDraw.Draw(horizontal).rectangle(
            (left, center_y, left + visible_length - 1, center_y + LINE_THICKNESS - 1),
            fill=stroke_index,
        )
        output = ASSET_DIRECTORY / f"{asset_name}_line_{suffix}"
        horizontal.save(output.with_suffix(".bmp"))
        write_sprite_metadata(output.with_suffix(".json"), width, height)

    for suffix, width, height, visible_length in VERTICAL_SEGMENTS:
        vertical = Image.new("P", (width, height), transparent_index)
        vertical.putpalette(palette)
        top = (height - visible_length) // 2
        center_x = width // 2
        ImageDraw.Draw(vertical).rectangle(
            (center_x, top, center_x + LINE_THICKNESS - 1, top + visible_length - 1),
            fill=stroke_index,
        )
        output = ASSET_DIRECTORY / f"{asset_name}_vertical_line_{suffix}"
        vertical.save(output.with_suffix(".bmp"))
        write_sprite_metadata(output.with_suffix(".json"), width, height)


for name in ASSET_NAMES:
    generate_line_assets(name)
