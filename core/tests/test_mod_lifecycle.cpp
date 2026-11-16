#include <okapy/mod_lifecycle.hpp>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

okapy::ModLoadPlan makePlan(std::vector<std::string> order) {
    okapy::ModLoadPlan plan;
    plan.dependencies.loadOrder = std::move(order);
    return plan;
}

void testStartAndReverseShutdown() {
    okapy::ModLifecycleManager lifecycle;
    std::vector<std::string> calls;

    assert(lifecycle.registerModule("example.base", [&] {
        calls.push_back("start-base"); return true;
    }, [&] { calls.push_back("stop-base"); }) == okapy::LifecycleResult::Started);

    assert(lifecycle.registerModule("example.addon", [&] {
        calls.push_back("start-addon"); return true;
    }, [&] { calls.push_back("stop-addon"); }) == okapy::LifecycleResult::Started);

    assert(lifecycle.start(makePlan({"example.base", "example.addon"}))
           == okapy::LifecycleResult::Started);
    assert(lifecycle.runningCount() == 2);
    assert(lifecycle.isRunning("example.base"));
    assert(lifecycle.isRunning("example.addon"));

    assert(lifecycle.stopAll() == okapy::LifecycleResult::Started);
    assert(lifecycle.runningCount() == 0);
    assert((calls == std::vector<std::string>{
        "start-base", "start-addon", "stop-addon", "stop-base"}));
}

void testFailedInitializerRollsBack() {
    okapy::ModLifecycleManager lifecycle;
    std::vector<std::string> calls;

    lifecycle.registerModule("example.first", [&] {
        calls.push_back("start-first"); return true;
    }, [&] { calls.push_back("stop-first"); });

    lifecycle.registerModule("example.second", [&] {
        calls.push_back("start-second"); return false;
    });

    assert(lifecycle.start(makePlan({"example.first", "example.second"}))
           == okapy::LifecycleResult::InitializationFailed);
    assert(lifecycle.runningCount() == 0);
    assert((calls == std::vector<std::string>{
        "start-first", "start-second", "stop-first"}));

    const auto entries = lifecycle.list();
    assert(entries.size() == 2);
    assert(entries[0].state == okapy::ModLifecycleState::Stopped);
    assert(entries[1].state == okapy::ModLifecycleState::Failed);
    assert(!entries[1].error.empty());
}

void testInvalidPlansAndRegistration() {
    okapy::ModLifecycleManager lifecycle;
    assert(lifecycle.registerModule("Bad.ID", [] { return true; })
           == okapy::LifecycleResult::InvalidId);
    assert(lifecycle.registerModule("example.noinit", {})
           == okapy::LifecycleResult::MissingInitializer);
    assert(lifecycle.registerModule("example.mod", [] { return true; })
           == okapy::LifecycleResult::Started);
    assert(lifecycle.registerModule("example.mod", [] { return true; })
           == okapy::LifecycleResult::DuplicateId);
    assert(lifecycle.start(makePlan({}))
           == okapy::LifecycleResult::InvalidLoadPlan);
    assert(lifecycle.start(makePlan({"example.missing"}))
           == okapy::LifecycleResult::MissingModule);
}

void testCallbackExceptions() {
    okapy::ModLifecycleManager lifecycle;
    lifecycle.registerModule("example.throw", []() -> bool {
        throw std::runtime_error("init failed");
    });
    assert(lifecycle.start(makePlan({"example.throw"}))
           == okapy::LifecycleResult::InitializationFailed);
    const auto entries = lifecycle.list();
    assert(entries.size() == 1);
    assert(entries[0].state == okapy::ModLifecycleState::Failed);
    assert(entries[0].error == "init failed");
}

} // namespace

int main() {
    testStartAndReverseShutdown();
    testFailedInitializerRollsBack();
    testInvalidPlansAndRegistration();
    testCallbackExceptions();
    return 0;
}
