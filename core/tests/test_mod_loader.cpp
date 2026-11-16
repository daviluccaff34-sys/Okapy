#include <okapy/mod_loader.hpp>

#include <cassert>

namespace {

okapy::ModInfo makeMod(const char* id, const char* version) {
    return {id, id, version, "test", "", okapy::ModState::Enabled};
}

void testDependencyFirstPlan() {
    okapy::ModRegistry registry;
    assert(registry.add(makeMod("example.base", "1.0.0")) == okapy::RegistryResult::Added);
    assert(registry.add(makeMod("example.addon", "1.0.0")) == okapy::RegistryResult::Added);

    std::vector<okapy::ModManifest> manifests{
        {"example.addon", {{"example.base", "1.0.0", false}}, {}},
        {"example.base", {}, {}}
    };

    const auto plan = okapy::ModLoader{}.prepare(registry, manifests);
    assert(!plan.dependencies.hasErrors());
    assert(plan.dependencies.loadOrder.size() == 2);
    assert(plan.dependencies.loadOrder[0] == "example.base");
    assert(plan.dependencies.loadOrder[1] == "example.addon");
}

void testInvalidDependenciesPreventReadyPlan() {
    okapy::ModRegistry registry;
    assert(registry.add(makeMod("example.addon", "1.0.0")) == okapy::RegistryResult::Added);

    std::vector<okapy::ModManifest> manifests{
        {"example.addon", {{"example.missing", "", false}}, {}}
    };

    const auto plan = okapy::ModLoader{}.prepare(registry, manifests);
    assert(plan.dependencies.hasErrors());
    assert(plan.dependencies.loadOrder.empty());
    assert(!plan.isReady());
}

void testEmptyPlanIsNotReady() {
    okapy::ModRegistry registry;
    const auto plan = okapy::ModLoader{}.prepare(registry, {});
    assert(!plan.isReady());
    assert(plan.dependencies.loadOrder.empty());
}

} // namespace

int main() {
    testDependencyFirstPlan();
    testInvalidDependenciesPreventReadyPlan();
    testEmptyPlanIsNotReady();
    return 0;
}
