#include <okapy/installed_package_store.hpp>
#include <okapy/package_manifest.hpp>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <system_error>
#include <utility>

namespace okapy {
namespace {

std::atomic<unsigned long long> g_removalSequence{1};
constexpr std::size_t kMaxManifestBytes = 64 * 1024;

bool isDirectPackageDirectory(
    const std::filesystem::path& root,
    const std::filesystem::path& candidate,
    std::filesystem::path& canonical) {
    std::error_code error;
    const auto status = std::filesystem::symlink_status(candidate, error);
    if (error || std::filesystem::is_symlink(status) ||
        !std::filesystem::is_directory(status)) {
        return false;
    }

    canonical = std::filesystem::canonical(candidate, error);
    if (error) return false;
    const auto canonicalRoot = std::filesystem::canonical(root, error);
    if (error || canonical.parent_path() != canonicalRoot) return false;

    const auto manifestPath = canonical / "okapy.json";
    const auto manifestStatus = std::filesystem::symlink_status(manifestPath, error);
    return !error && !std::filesystem::is_symlink(manifestStatus) &&
        std::filesystem::is_regular_file(manifestStatus);
}

} // namespace

InstalledPackageStore::InstalledPackageStore(InstalledModCatalog& catalog) noexcept
    : m_catalog(catalog) {}

bool InstalledPackageStore::setRoot(std::string rootPath) noexcept {
    if (rootPath.empty()) return false;
    try {
        std::error_code error;
        std::filesystem::path candidate(std::move(rootPath));
        std::filesystem::create_directories(candidate, error);
        if (error || !std::filesystem::is_directory(candidate, error) || error) {
            return false;
        }
        auto canonical = std::filesystem::canonical(candidate, error);
        if (error) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_root = canonical.string();
        return true;
    } catch (...) {
        return false;
    }
}

std::string InstalledPackageStore::root() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_root;
}

std::vector<std::string> InstalledPackageStore::listPackageIds() const {
    const auto rootPath = root();
    if (rootPath.empty()) return {};

    std::vector<std::string> ids;
    std::error_code error;
    const std::filesystem::path rootDirectory(rootPath);
    for (std::filesystem::directory_iterator it(rootDirectory, error), end;
         !error && it != end; it.increment(error)) {
        const auto id = it->path().filename().string();
        if (!ModRegistry::isValidId(id)) continue;
        std::filesystem::path canonical;
        if (isDirectPackageDirectory(rootDirectory, it->path(), canonical)) {
            ids.push_back(id);
        }
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::string InstalledPackageStore::packagePath(std::string_view modId) const {
    if (!ModRegistry::isValidId(modId)) return {};
    const auto rootPath = root();
    if (rootPath.empty()) return {};

    const std::filesystem::path rootDirectory(rootPath);
    std::filesystem::path canonical;
    if (!isDirectPackageDirectory(
            rootDirectory, rootDirectory / std::string(modId), canonical)) {
        return {};
    }
    return canonical.string();
}

std::vector<PackageDiscovery> InstalledPackageStore::discoverInstalled() {
    std::vector<PackageDiscovery> discoveries;
    const auto ids = listPackageIds();
    discoveries.reserve(ids.size());

    for (const auto& directoryId : ids) {
        PackageDiscovery discovery;
        discovery.packageId = directoryId;
        const auto directory = packagePath(directoryId);
        if (directory.empty()) {
            discovery.result = PackageStoreResult::UnsafePath;
            discovery.error = "Package directory failed canonical path validation.";
            discoveries.push_back(std::move(discovery));
            continue;
        }

        const auto manifestPath = std::filesystem::path(directory) / "okapy.json";
        std::error_code error;
        const auto fileSize = std::filesystem::file_size(manifestPath, error);
        if (error || fileSize == 0 || fileSize > kMaxManifestBytes) {
            discovery.result = PackageStoreResult::InvalidManifest;
            discovery.error = "Manifest is unreadable, empty, or exceeds 64 KiB.";
            discoveries.push_back(std::move(discovery));
            continue;
        }

        std::ifstream input(manifestPath, std::ios::binary);
        if (!input) {
            discovery.result = PackageStoreResult::InvalidManifest;
            discovery.error = "Manifest could not be opened.";
            discoveries.push_back(std::move(discovery));
            continue;
        }
        std::string contents((std::istreambuf_iterator<char>(input)),
                             std::istreambuf_iterator<char>());
        if (contents.size() != fileSize) {
            discovery.result = PackageStoreResult::InvalidManifest;
            discovery.error = "Manifest read was incomplete.";
            discoveries.push_back(std::move(discovery));
            continue;
        }

        auto parsed = parsePackageManifest(contents, directory);
        if (!parsed.valid || parsed.mod.id != directoryId) {
            discovery.result = PackageStoreResult::InvalidManifest;
            discovery.error = parsed.valid
                ? "Manifest ID does not match its installed directory."
                : std::move(parsed.error);
            discoveries.push_back(std::move(discovery));
            continue;
        }

        if (m_catalog.manifest(directoryId)) {
            discovery.result = PackageStoreResult::AlreadyRegistered;
            discoveries.push_back(std::move(discovery));
            continue;
        }

        discovery.result = registerInstalled(
            std::move(parsed.mod), std::move(parsed.dependencies));
        if (discovery.result == PackageStoreResult::CatalogRejected) {
            discovery.error = "The installed mod catalog rejected this manifest.";
        }
        discoveries.push_back(std::move(discovery));
    }
    return discoveries;
}

PackageStoreResult InstalledPackageStore::registerInstalled(
    ModInfo mod, ModManifest manifest) {
    if (!ModRegistry::isValidId(mod.id)) return PackageStoreResult::InvalidMod;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_root.empty()) return PackageStoreResult::StorageError;
    const std::filesystem::path rootDirectory(m_root);
    std::filesystem::path canonical;
    if (!isDirectPackageDirectory(
            rootDirectory, rootDirectory / mod.id, canonical)) {
        return PackageStoreResult::MissingPackage;
    }

    mod.packagePath = canonical.string();
    const auto result = m_catalog.install(std::move(mod), std::move(manifest));
    if (result == CatalogResult::Added) return PackageStoreResult::Registered;
    if (result == CatalogResult::InvalidMod) return PackageStoreResult::InvalidMod;
    return PackageStoreResult::CatalogRejected;
}

PackageStoreResult InstalledPackageStore::uninstall(std::string_view modId) {
    if (!ModRegistry::isValidId(modId)) return PackageStoreResult::InvalidMod;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_root.empty()) return PackageStoreResult::StorageError;
    if (!m_catalog.manifest(modId)) return PackageStoreResult::NotFound;
    if (m_catalog.isEnabled(modId)) return PackageStoreResult::ModEnabled;

    const std::filesystem::path rootDirectory(m_root);
    std::filesystem::path packageDirectory;
    if (!isDirectPackageDirectory(
            rootDirectory, rootDirectory / std::string(modId), packageDirectory)) {
        return PackageStoreResult::MissingPackage;
    }

    const auto tombstoneName = std::string(".okapy-remove-") +
        std::string(modId) + "-" +
        std::to_string(g_removalSequence.fetch_add(1, std::memory_order_relaxed));
    const auto tombstone = rootDirectory / tombstoneName;

    std::error_code error;
    std::filesystem::rename(packageDirectory, tombstone, error);
    if (error) return PackageStoreResult::StorageError;

    const auto catalogResult = m_catalog.uninstall(modId);
    if (catalogResult != CatalogResult::Removed) {
        std::error_code rollbackError;
        std::filesystem::rename(tombstone, packageDirectory, rollbackError);
        return rollbackError ? PackageStoreResult::StorageError
                             : (catalogResult == CatalogResult::ModEnabled
                                    ? PackageStoreResult::ModEnabled
                                    : PackageStoreResult::CatalogRejected);
    }

    std::filesystem::remove_all(tombstone, error);
    return error ? PackageStoreResult::RemovedWithCleanupPending
                 : PackageStoreResult::Removed;
}

} // namespace okapy
