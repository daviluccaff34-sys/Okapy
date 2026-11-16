#include <okapy/mod_loader.hpp>

namespace okapy {

ModLoadPlan ModLoader::prepare(
    const ModRegistry& registry,
    const std::vector<ModManifest>& manifests) const {
    ModLoadPlan plan;
    plan.dependencies = m_dependencyManager.resolve(registry, manifests);
    return plan;
}

} // namespace okapy
