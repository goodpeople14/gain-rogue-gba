#include "world/stage_definition.h"

#include "bn_assert.h"

#include "world/stages/stage1.h"
#include "world/stages/stage2.h"

const StageDefinition& stage_definition(StageId stage)
{
    switch(stage)
    {
    case StageId::STAGE_1:
        return stage1::definition;

    case StageId::STAGE_2:
        return stage2::definition;

    default:
        BN_ASSERT(false, "Unknown stage");
        return stage1::definition;
    }
}
