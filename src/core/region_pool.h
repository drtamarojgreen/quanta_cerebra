#ifndef REGION_POOL_H
#define REGION_POOL_H

#include "brain_region.h"
#include <vector>

class RegionPool {
    std::vector<BrainRegion*> pool;
public:
    ~RegionPool();
    BrainRegion* acquire();
    void release(BrainRegion* r);
};

#endif
