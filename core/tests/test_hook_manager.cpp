#include <okapy/hook_manager.hpp>

#include <cassert>

namespace {

okapy::HookInfo makeHook(const char* id, const char* owner,
                         std::uintptr_t target, std::uintptr_t detour) {
    return {id, owner, target, detour, okapy::HookState::Registered};
}

void testIdValidation() {
    assert(okapy::HookManager::isValidId("example.hook"));
    assert(okapy::HookManager::isValidId("hook_01"));
    assert(!okapy::HookManager::isValidId("Example.Hook"));
    assert(!okapy::HookManager::isValidId("a..b"));
    assert(!okapy::HookManager::isValidId(".hook"));
}

void testAddAndFind() {
    okapy::HookManager manager;
    assert(manager.add(makeHook("example.first", "example.mod", 0x1000, 0x2000))
           == okapy::HookResult::Added);
    const auto hook = manager.find("example.first");
    assert(hook.has_value());
    assert(hook->targetAddress == 0x1000);
    assert(hook->state == okapy::HookState::Registered);
    assert(manager.size() == 1);
}

void testRejectInvalidAndDuplicateHooks() {
    okapy::HookManager manager;
    assert(manager.add(makeHook("Bad.ID", "example.mod", 0x1000, 0x2000))
           == okapy::HookResult::InvalidId);
    assert(manager.add(makeHook("example.first", "Bad.Owner", 0x1000, 0x2000))
           == okapy::HookResult::InvalidOwner);
    assert(manager.add(makeHook("example.first", "example.mod", 0, 0x2000))
           == okapy::HookResult::InvalidAddress);
    assert(manager.add(makeHook("example.first", "example.mod", 0x1000, 0x2000))
           == okapy::HookResult::Added);
    assert(manager.add(makeHook("example.first", "other.mod", 0x3000, 0x4000))
           == okapy::HookResult::DuplicateId);
    assert(manager.add(makeHook("example.second", "other.mod", 0x1000, 0x5000))
           == okapy::HookResult::TargetConflict);
}

void testStateAndOwnerDisable() {
    okapy::HookManager manager;
    assert(manager.add(makeHook("mod.one", "example.mod", 0x1000, 0x2000))
           == okapy::HookResult::Added);
    assert(manager.add(makeHook("mod.two", "example.mod", 0x3000, 0x4000))
           == okapy::HookResult::Added);
    assert(manager.add(makeHook("other.one", "other.mod", 0x5000, 0x6000))
           == okapy::HookResult::Added);

    assert(manager.setEnabled("mod.one", true));
    assert(manager.setEnabled("mod.two", true));
    assert(manager.setEnabled("other.one", true));
    assert(manager.disableOwner("example.mod") == 2);
    assert(manager.find("mod.one")->state == okapy::HookState::Disabled);
    assert(manager.find("other.one")->state == okapy::HookState::Enabled);
    assert(!manager.setEnabled("missing.hook", true));
}

void testListRemoveAndClear() {
    okapy::HookManager manager;
    manager.add(makeHook("z.last", "example.mod", 0x1000, 0x2000));
    manager.add(makeHook("a.first", "example.mod", 0x3000, 0x4000));
    const auto hooks = manager.list();
    assert(hooks.size() == 2);
    assert(hooks.front().id == "a.first");
    assert(manager.remove("a.first"));
    assert(!manager.remove("a.first"));
    manager.clear();
    assert(manager.size() == 0);
}

} // namespace

int main() {
    testIdValidation();
    testAddAndFind();
    testRejectInvalidAndDuplicateHooks();
    testStateAndOwnerDisable();
    testListRemoveAndClear();
    return 0;
}
