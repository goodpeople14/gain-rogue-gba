#ifndef WORLD_STAGE_DATA_H
#define WORLD_STAGE_DATA_H

#include <stdint.h>

enum class StageCell : uint8_t
{
    WALKABLE,
    BLOCKED
};

struct StageData
{
    int width;
    int height;
    int tile_size;
    const StageCell* movement_cells;
};

[[nodiscard]] constexpr bool stage_cell_in_bounds(const StageData& stage, int cell_x, int cell_y)
{
    return cell_x >= 0 && cell_x < stage.width && cell_y >= 0 && cell_y < stage.height;
}

[[nodiscard]] constexpr int stage_cell_index(const StageData& stage, int cell_x, int cell_y)
{
    return (cell_y * stage.width) + cell_x;
}

[[nodiscard]] constexpr StageCell stage_cell_at(const StageData& stage, int cell_x, int cell_y)
{
    return stage.movement_cells[stage_cell_index(stage, cell_x, cell_y)];
}

[[nodiscard]] constexpr int stage_world_minimum(const StageData& stage)
{
    return -(stage.width * stage.tile_size) / 2;
}

[[nodiscard]] constexpr int stage_cell_from_world_coordinate(const StageData& stage, int world_coordinate)
{
    int stage_local_coordinate = world_coordinate - stage_world_minimum(stage);
    return stage_local_coordinate >= 0 ? stage_local_coordinate / stage.tile_size :
           -(((-stage_local_coordinate) + stage.tile_size - 1) / stage.tile_size);
}

#endif
