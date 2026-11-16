#include <okapy/installed_mod_catalog.hpp>

#include <cassert>

namespace {
okapy::ModInfo mod(const char* id, const char* version) {
    return {id, id, version, "test developer", "/mods/test", okapy::ModState::Discovered};
}
}

int main() {
    okapy::ModRegistry registry;
    okapy::InstalledModCatalog catalog(registry);

    const okapy::ModManifest base{"example.base", {}, {}};
    const okapy::ModManifest addon{
        "example.addon", {{"example.base", "1.0.0", false}}, {}
    };

    assert(catalog.install(mod("example.base", "1.0.0"), base) ==
        okapy::CatalogResult::Added);
    assert(catalog.install(mod("example.addon", "1.2.0"), addon) ==
        okapy::CatalogResult::Added);
    assert(registry.size() == 2);
    assert(registry.find("example.base")->state == okapy::ModState::Installed);

    assert(catalog.install(mod("example.base", "1.0.0"), base) ==
        okapy::CatalogResult::AlreadyExists);
    assert(catalog.install(mod("example.other", "1.0.0"),
        okapy::ModManifest{"wrong.id", {}, {}}) ==
        okapy::CatalogResult::ManifestMismatch);

    assert(catalog.setEnabled("example.base", true));
    assert(catalog.setEnabled("example.addon", true));
    auto report = catalog.resolveEnabled();
    assert(!report.hasErrors());
    assert(report.loadOrder.size() == 2);
    assert(report.loadOrder[0] == "example.base");
    assert(report.loadOrder[1] == "example.addon");

    assert(catalog.update(mod("example.base", "2.0.0"), base) ==
        okapy::CatalogResult::Updated);
    assert(registry.find("example.base")->version == "2.0.0");
    assert(registry.find("example.base")->state == okapy::ModState::Installed);
    assert(catalog.resolveEnabled().hasErrors());

    assert(catalog.setEnabled("example.base", true));
    report = catalog.resolveEnabled();
    assert(!report.hasErrors());

    assert(catalog.manifests().size() == 2);
    assert(catalog.manifest("example.addon").has_value());

    assert(catalog.uninstall("example.base") == okapy::CatalogResult::Removed);
    assert(!registry.find("example.base").has_value());
    assert(!catalog.manifest("example.base").has_value());
    report = catalog.resolveEnabled();
    assert(report.hasErrors());
    assert(catalog.uninstall("example.base") == okapy::CatalogResult::NotFound);
    assert(catalog.uninstall("../bad") == okapy::CatalogResult::InvalidMod);
    return 0;
}
