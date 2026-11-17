#pragma once

#include <okapy/installed_mod_catalog.hpp>

#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace okapy {

enum class PackageStoreResult {
    Registered,
    AlreadyRegistered,
    Removed,
    RemovedWithCleanupPending,
    NotFound,
    InvalidMod,
    InvalidManifest,
    MissingPackage,
    UnsafePath,
    ModEnabled,
    CatalogRejected,
    StorageError
};

struct PackageDiscovery {
    std::string packageId;
    PackageStoreResult result = PackageStoreResult::StorageError;
    std::string error;
};

// Filesystem boundary for already-extracted .okapy package directories.
// It parses root manifests but does not inspect ZIP archives or load code.
class InstalledPackageStore final {
public:
    explicit InstalledPackageStore(InstalledModCatalog& catalog) noexcept;

    bool setRoot(std::string rootPath) noexcept;
    [[nodiscard]] std::string root() const;
    [[nodiscard]] std::vector<std::string> listPackageIds() const;
    [[nodiscard]] std::string packagePath(std::string_view modId) const;
    [[nodiscard]] std::vector<PackageDiscovery> discoverInstalled();

    PackageStoreResult registerInstalled(ModInfo mod, ModManifest manifest);
    PackageStoreResult uninstall(std::string_view modId);

private:
    InstalledModCatalog& m_catalog;
    mutable std::mutex m_mutex;
    std::string m_root;
};

} // namespace okapy
