#ifndef BRAIN_REGION_H
#define BRAIN_REGION_H

#include "core/atlas_region.h"
#include "core/state_manager.h"

// Bring types into global scope if needed for legacy compatibility, 
// but preferred to use cerebra::
using BrainRegion = cerebra::RegionState;
using cerebra::BrainFrame = cerebra::BrainFrame;

#endif
