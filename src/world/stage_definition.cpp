#include "world/stage_definition.h"

#include "bn_assert.h"

#include "world/stages/stage1.h"
#include "world/stages/stage2.h"
#include "world/stages/stage3.h"
#include "world/stages/stage4.h"
#include "world/stages/stage5.h"

const StageDefinition& stage_definition(StageId stage)
{
    switch(stage)
    {
    case StageId::STAGE_1:
        return stage1::definition;

    case StageId::STAGE_2:
        return stage2::definition;

    case StageId::STAGE_3:
        return stage3::definition;

    case StageId::STAGE_4:
        return stage4::definition;

    case StageId::STAGE_5:
        return stage5::definition;

    default:
        BN_ASSERT(false, "Unknown stage");
        return stage1::definition;
    }
}
