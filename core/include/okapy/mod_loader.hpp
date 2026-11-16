#pragma once

#include <okapy/dependency_manager.hpp>

namespace okapy {

struct ModLoadPlan {
    DependencyReport dependencies;

    [[nodiscard]] bool isReady() const noexcept {
        return !dependencies.hasErrors() && !dependencies.loadOrder.empty();
    }
};

// Coordinates metadata validation and dependency ordering. It does not execute mod code.
class ModLoader final {
public:
    [[nodiscard]] ModLoadPlan prepare(
        const ModRegistry& registry,
        const std::vector<ModManifest>& manifests) const;

private:
    DependencyManager m_dependencyManager;
};

} // namespace okapy
