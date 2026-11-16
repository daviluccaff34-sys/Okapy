#pragma once

#include <okapy/installed_mod_catalog.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace okapy {

enum class PackageStoreResult {
    Registered,
    Removed,
    RemovedWithCleanupPending,
    NotFound,
    InvalidMod,
    MissingPackage,
    UnsafePath,
    ModEnabled,
    CatalogRejected,
    StorageError
};

// Filesystem boundary for already-extracted .okapy package directories.
// Manifest parsing and archive extraction remain the responsibility of the
// package-validation/installation layer; metadata is supplied by that layer.
class InstalledPackageStore final {
public:
    explicit InstalledPackageStore(InstalledModCatalog& catalog) noexcept;

    bool setRoot(std::string rootPath) noexcept;
    [[nodiscard]] std::string root() const;
    [[nodiscard]] std::vector<std::string> listPackageIds() const;
    [[nodiscard]] std::string packagePath(std::string_view modId) const;

    PackageStoreResult registerInstalled(ModInfo mod, ModManifest manifest);
    PackageStoreResult uninstall(std::string_view modId);

private:
    InstalledModCatalog& m_catalog;
    std::string m_root;
};

} // namespace okapy
