#pragma once

#include <okapy/dependency_manager.hpp>

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace okapy {

enum class CatalogResult {
    Added,
    Updated,
    Removed,
    NotFound,
    InvalidMod,
    ManifestMismatch,
    AlreadyExists,
    ModEnabled
};

// In-memory catalog joining installed package metadata to dependency manifests.
// It does not parse archives, touch files, or load native code.
class InstalledModCatalog final {
public:
    explicit InstalledModCatalog(ModRegistry& registry) noexcept;

    CatalogResult install(ModInfo mod, ModManifest manifest);
    CatalogResult update(ModInfo mod, ModManifest manifest);
    CatalogResult uninstall(std::string_view modId);
    bool setEnabled(std::string_view modId, bool enabled);

    [[nodiscard]] std::vector<ModManifest> manifests() const;
    [[nodiscard]] std::optional<ModManifest> manifest(std::string_view modId) const;
    [[nodiscard]] DependencyReport resolveEnabled() const;

private:
    static bool isValidManifest(const ModInfo& mod,
                                const ModManifest& manifest) noexcept;

    ModRegistry& m_registry;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ModManifest> m_manifests;
    DependencyManager m_dependencyManager;
};

} // namespace okapy
