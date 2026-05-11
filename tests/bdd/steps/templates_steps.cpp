#include "gherkin_runner.hpp"

#include "core/state_manager.h"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using TemplateMap = std::map<std::string, cerebra::BrainFrame>;

double intensity_of(const cerebra::BrainFrame& f, const std::string& region) {
    for (const auto& r : f.regions) {
        if (r.region == region) return r.intensity;
    }
    throw std::runtime_error("region not present in frame: " + region);
}

const cerebra::BrainFrame& lookup(bm_bdd::Context& ctx, const std::string& name) {
    auto& m = ctx.get<TemplateMap>("templates");
    auto it = m.find(name);
    if (it == m.end()) throw std::runtime_error("unknown template: " + name);
    return it->second;
}

}

GIVEN("the four built-in brain-state templates") {
    TemplateMap m;
    m["focused"]   = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    m["relaxed"]   = cerebra::template_frame(cerebra::BrainTemplate::Relaxed, 0);
    m["stressed"] = cerebra::template_frame(cerebra::BrainTemplate::Stressed, 0);
    m["rem_sleep"] = cerebra::template_frame(cerebra::BrainTemplate::REMSleep, 0);
    ctx.set<TemplateMap>("templates", std::move(m));
}

THEN("{string} {string} intensity is greater than {string} {string} intensity") {
    double a = intensity_of(lookup(ctx, args[0]), args[1]);
    double b = intensity_of(lookup(ctx, args[2]), args[3]);
    if (!(a > b)) {
        std::ostringstream oss;
        oss << args[0] << "." << args[1] << "=" << a << " not > "
            << args[2] << "." << args[3] << "=" << b;
        throw std::runtime_error(oss.str());
    }
}

THEN("{string} {string} intensity exceeds {string} {string} intensity by at least {float}") {
    double a = intensity_of(lookup(ctx, args[0]), args[1]);
    double b = intensity_of(lookup(ctx, args[2]), args[3]);
    double margin = std::stod(args[4]);
    if (!(a - b >= margin)) {
        std::ostringstream oss;
        oss << args[0] << "." << args[1] << "=" << a << " minus "
            << args[2] << "." << args[3] << "=" << b << " < " << margin;
        throw std::runtime_error(oss.str());
    }
}

THEN("{string} {string} intensity is greater than {float}") {
    double v = intensity_of(lookup(ctx, args[0]), args[1]);
    double threshold = std::stod(args[2]);
    if (!(v > threshold)) {
        std::ostringstream oss;
        oss << args[0] << "." << args[1] << "=" << v << " not > " << threshold;
        throw std::runtime_error(oss.str());
    }
}
