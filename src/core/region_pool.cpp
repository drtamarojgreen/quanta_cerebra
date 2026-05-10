#include "region_pool.h"

RegionPool::~RegionPool() {
    for (BrainRegion* r : pool) delete r;
    pool.clear();
}

BrainRegion* RegionPool::acquire() {
    if(pool.empty()) return new BrainRegion();
    BrainRegion* r = pool.back(); pool.pop_back(); return r;
}

void RegionPool::release(BrainRegion* r) {
    if (r) {
        r->subregions.clear();
        r->intensity_history.clear();
        r->synaptic_buffer.clear();
        r->neurotransmitters.clear();
        r->synapses.clear();
        pool.push_back(r);
    }
}
