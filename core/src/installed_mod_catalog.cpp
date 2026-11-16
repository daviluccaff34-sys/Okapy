#include <okapy/installed_mod_catalog.hpp>

#include <algorithm>
#include <utility>

namespace okapy {

InstalledModCatalog::InstalledModCatalog(ModRegistry& registry) noexcept
    : m_registry(registry) {}

bool InstalledModCatalog::isValidManifest(
    const ModInfo& mod, const ModManifest& manifest) noexcept {
    if (!ModRegistry::isValidId(mod.id) || manifest.id != mod.id ||
        mod.name.empty() || mod.version.empty()) {
        return false;
    }
    for (const auto& dependency : manifest.dependencies) {
        if (!ModRegistry::isValidId(dependency.id) || dependency.id == mod.id) {
            return false;
        }
    }
    for (const auto& conflict : manifest.conflicts) {
        if (!ModRegistry::isValidId(conflict) || conflict == mod.id) {
            return false;
        }
    }
    return true;
}

CatalogResult InstalledModCatalog::install(ModInfo mod, ModManifest manifest) {
    if (!isValidManifest(mod, manifest)) return CatalogResult::ManifestMismatch;
    mod.state = ModState::Installed;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_manifests.find(mod.id) != m_manifests.end()) {
        return CatalogResult::AlreadyExists;
    }
    const auto id = mod.id;
    const auto added = m_registry.add(std::move(mod));
    if (added == RegistryResult::InvalidId) return CatalogResult::InvalidMod;
    if (added == RegistryResult::DuplicateId) return CatalogResult::AlreadyExists;
    m_manifests.emplace(id, std::move(manifest));
    return CatalogResult::Added;
}

CatalogResult InstalledModCatalog::update(ModInfo mod, ModManifest manifest) {
    if (!isValidManifest(mod, manifest)) return CatalogResult::ManifestMismatch;

    std::lock_guard<std::mutex> lock(m_mutex);
    const auto found = m_manifests.find(mod.id);
    if (found == m_manifests.end()) return CatalogResult::NotFound;

    const auto current = m_registry.find(mod.id);
    if (!current) return CatalogResult::NotFound;
    if (current->state == ModState::Enabled) return CatalogResult::ModEnabled;

    // Updating a package returns it to Installed; it must be explicitly enabled again.
    mod.state = ModState::Installed;
    if (!m_registry.replace(mod)) return CatalogResult::NotFound;
    found->second = std::move(manifest);
    return CatalogResult::Updated;
}

CatalogResult InstalledModCatalog::uninstall(std::string_view modId) {
    if (!ModRegistry::isValidId(modId)) return CatalogResult::InvalidMod;
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto found = m_manifests.find(std::string(modId));
    if (found == m_manifests.end()) return CatalogResult::NotFound;

    const auto current = m_registry.find(modId);
    if (!current) return CatalogResult::NotFound;
    if (current->state == ModState::Enabled) return CatalogResult::ModEnabled;

    if (!m_registry.remove(modId)) return CatalogResult::NotFound;
    m_manifests.erase(found);
    return CatalogResult::Removed;
}

bool InstalledModCatalog::setEnabled(std::string_view modId, bool enabled) {
    if (!ModRegistry::isValidId(modId)) return false;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_manifests.find(std::string(modId)) == m_manifests.end()) return false;
    return m_registry.setState(modId, enabled ? ModState::Enabled : ModState::Disabled);
}

std::vector<ModManifest> InstalledModCatalog::manifests() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ModManifest> result;
    result.reserve(m_manifests.size());
    for (const auto& entry : m_manifests) result.push_back(entry.second);
    std::sort(result.begin(), result.end(),
        [](const ModManifest& left, const ModManifest& right) {
            return left.id < right.id;
        });
    return result;
}

std::optional<ModManifest> InstalledModCatalog::manifest(std::string_view modId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto found = m_manifests.find(std::string(modId));
    if (found == m_manifests.end()) return std::nullopt;
    return found->second;
}

DependencyReport InstalledModCatalog::resolveEnabled() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<ModManifest> snapshot;
    snapshot.reserve(m_manifests.size());
    for (const auto& entry : m_manifests) snapshot.push_back(entry.second);
    return m_dependencyManager.resolve(m_registry, snapshot);
}

} // namespace okapy
