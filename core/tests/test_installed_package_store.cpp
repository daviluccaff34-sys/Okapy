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
        std::ofstream manifest(root / "example.mod" / "okapy.json");
        manifest << R"json({
            "formatVersion": 1,
            "id": "example.mod",
            "name": "Example",
            "version": "1.0.0",
            "developer": "Test author",
            "game": {"id": "com.robtopx.geometryjump", "versions": ["2.2"]},
            "entrypoints": {"android": {"arm64-v8a": "libs/libexample.so"}},
            "dependencies": [],
            "conflicts": []
        })json";
        assert(manifest.good());
    }
    {
        std::ofstream library(root / "example.mod" / "libs" / "libexample.so",
                              std::ios::binary);
        library << "not a real shared library; path validation only";
        assert(library.good());
    }

    okapy::ModRegistry registry;
    okapy::InstalledModCatalog catalog(registry);
    okapy::InstalledPackageStore store(catalog);

    assert(store.setRoot(root.string()));
    assert(store.root() == fs::canonical(root).string());
    assert(store.listPackageIds().size() == 1);
    assert(store.listPackageIds().front() == "example.mod");
    assert(store.packagePath("../outside").empty());
    assert(store.entrypointPath("example.mod", "arm64-v8a") ==
           fs::canonical(root / "example.mod" / "libs" / "libexample.so").string());
    assert(store.entrypointPath("example.mod", "x86").empty());

    const auto discoveries = store.discoverInstalled();
    assert(discoveries.size() == 1);
    assert(discoveries.front().result == okapy::PackageStoreResult::Registered);
    const auto registered = registry.find("example.mod");
    assert(registered.has_value());
    assert(registered->name == "Example");
    assert(registered->packagePath == fs::canonical(root / "example.mod").string());

    const auto repeatedScan = store.discoverInstalled();
    assert(repeatedScan.size() == 1);
    assert(repeatedScan.front().result == okapy::PackageStoreResult::AlreadyRegistered);

    fs::remove(root / "example.mod" / "libs" / "libexample.so", error);
    assert(!error);
    assert(store.entrypointPath("example.mod", "arm64-v8a").empty());
    {
        std::ofstream library(root / "example.mod" / "libs" / "libexample.so",
                              std::ios::binary);
        library << "restored";
        assert(library.good());
    }

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
