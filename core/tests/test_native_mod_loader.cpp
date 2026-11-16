#include <okapy/native_mod_loader.hpp>

#include <cassert>
#include <cstdint>

namespace {
int logCount = 0;
void testLog(void*, int32_t, const char*) { ++logCount; }
}

int main() {
    okapy::ModLifecycleManager lifecycle;
    okapy::NativeModLoader loader;
    const OkapyHostApiV1 host{
        sizeof(OkapyHostApiV1),
        OKAPY_MOD_ABI_VERSION_1,
        nullptr,
        testLog
    };

    okapy::NativeModLoadInfo info;
    const auto result = loader.load(
        OKAPY_ABI_TEST_MODULE_PATH, lifecycle, host, &info);
    assert(result == okapy::NativeModLoadResult::Registered);
    assert(info.id == "fixture.test");
    assert(info.name == "Okapy ABI Test Fixture");
    assert(loader.contains("fixture.test"));

    okapy::ModLoadPlan plan;
    plan.dependencies.loadOrder.push_back("fixture.test");
    assert(lifecycle.start(plan) == okapy::LifecycleResult::Started);
    assert(lifecycle.isRunning("fixture.test"));
    assert(logCount == 1);

    assert(lifecycle.stopAll() == okapy::LifecycleResult::Started);
    assert(!lifecycle.isRunning("fixture.test"));
    return 0;
}
