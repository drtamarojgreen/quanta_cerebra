#ifndef PLUGINS_H
#define PLUGINS_H
#include "json_logic.h"
#include <vector>
#include <memory>

// Enhancement 71: Plugin Architecture
class IRendererPlugin {
public:
    virtual ~IRendererPlugin() = default;
    virtual void render(const BrainFrame& frame) = 0;
};

class PluginManager {
    std::vector<std::unique_ptr<IRendererPlugin>> plugins;
public:
    void registerPlugin(std::unique_ptr<IRendererPlugin> p) { plugins.push_back(std::move(p)); }
    void notifyRender(const BrainFrame& frame) { for(auto& p : plugins) p->render(frame); }
};

#endif
