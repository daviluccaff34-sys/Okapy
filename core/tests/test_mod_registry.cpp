#include <okapy/mod_registry.hpp>

#include <cassert>

int main() {
    okapy::ModRegistry registry;

    assert(okapy::ModRegistry::isValidId("example.mod"));
    assert(okapy::ModRegistry::isValidId("mod_123"));
    assert(!okapy::ModRegistry::isValidId(""));
    assert(!okapy::ModRegistry::isValidId("Example.Mod"));
    assert(!okapy::ModRegistry::isValidId("one..two"));
    assert(!okapy::ModRegistry::isValidId(".mod"));

    okapy::ModInfo mod;
    mod.id = "example.mod";
    mod.name = "Example Mod";
    mod.version = "1.0.0";
    mod.developer = "Okapy Test";
    mod.state = okapy::ModState::Installed;

    assert(registry.add(mod) == okapy::RegistryResult::Added);
    assert(registry.add(mod) == okapy::RegistryResult::DuplicateId);
    assert(registry.size() == 1);

    const auto found = registry.find("example.mod");
    assert(found.has_value());
    assert(found->name == "Example Mod");

    assert(registry.setState("example.mod", okapy::ModState::Enabled));
    assert(registry.find("example.mod")->state == okapy::ModState::Enabled);
    assert(!registry.setState("missing.mod", okapy::ModState::Enabled));

    const auto mods = registry.list();
    assert(mods.size() == 1);
    assert(mods.front().id == "example.mod");

    assert(registry.remove("example.mod"));
    assert(!registry.find("example.mod").has_value());
    assert(!registry.remove("example.mod"));

    registry.clear();
    assert(registry.size() == 0);
    return 0;
}
