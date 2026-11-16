#include <okapy/installed_package_store.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>

int main() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "okapy-installed-package-store-test";
    std::error_code error;
    fs::remove_all(root, error);
    error.clear();
    fs::create_directories(root / "example.mod" / "libs", error);
    assert(!error);
    fs::create_directories(root / "empty.mod", error);
    assert(!error);

    {
        std::ofstream marker(root / "example.mod" / "okapy.json");
        marker << "{\"id\":\"example.mod\"}";
        assert(marker.good());
    }

    okapy::ModRegistry registry;
    okapy::InstalledModCatalog catalog(registry);
    okapy::InstalledPackageStore store(catalog);

    assert(store.setRoot(root.string()));
    assert(store.root() == fs::canonical(root).string());
    assert(store.listPackageIds().size() == 1);
    assert(store.listPackageIds().front() == "example.mod");
    assert(store.packagePath("../outside").empty());

    okapy::ModInfo info{
        "example.mod", "Example", "1.0.0", "Test author", "",
        okapy::ModState::Discovered
    };
    const okapy::ModManifest manifest{"example.mod", {}, {}};
    assert(store.registerInstalled(info, manifest) ==
        okapy::PackageStoreResult::Registered);
    const auto registered = registry.find("example.mod");
    assert(registered.has_value());
    assert(registered->packagePath == fs::canonical(root / "example.mod").string());

    assert(catalog.setEnabled("example.mod", true));
    assert(store.uninstall("example.mod") == okapy::PackageStoreResult::ModEnabled);
    assert(fs::is_directory(root / "example.mod"));

    assert(catalog.setEnabled("example.mod", false));
    assert(store.uninstall("example.mod") == okapy::PackageStoreResult::Removed);
    assert(!fs::exists(root / "example.mod"));
    assert(!registry.find("example.mod").has_value());
    assert(store.uninstall("example.mod") == okapy::PackageStoreResult::NotFound);
    assert(store.uninstall("../bad") == okapy::PackageStoreResult::InvalidMod);

    fs::remove_all(root, error);
    return 0;
}
