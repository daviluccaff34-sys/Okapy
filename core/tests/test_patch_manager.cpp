#include <okapy/patch_manager.hpp>

#include <cassert>
#include <limits>

namespace {

okapy::PatchInfo makePatch(const char* id, const char* owner,
                           std::uintptr_t address,
                           std::vector<std::uint8_t> expected = {0x10, 0x20},
                           std::vector<std::uint8_t> replacement = {0x30, 0x40}) {
    return {id, owner, address, std::move(expected), std::move(replacement),
            okapy::PatchState::Registered};
}

void testIdValidation() {
    assert(okapy::PatchManager::isValidId("example.patch"));
    assert(okapy::PatchManager::isValidId("patch_01"));
    assert(!okapy::PatchManager::isValidId("Example.Patch"));
    assert(!okapy::PatchManager::isValidId("a..b"));
    assert(!okapy::PatchManager::isValidId(".patch"));
}

void testAddAndFind() {
    okapy::PatchManager manager;
    assert(manager.add(makePatch("example.first", "example.mod", 0x1000))
           == okapy::PatchResult::Added);
    const auto patch = manager.find("example.first");
    assert(patch.has_value());
    assert(patch->targetAddress == 0x1000);
    assert(patch->expectedBytes.size() == 2);
    assert(patch->state == okapy::PatchState::Registered);
}

void testRejectInvalidPatches() {
    okapy::PatchManager manager;
    assert(manager.add(makePatch("Bad.ID", "example.mod", 0x1000))
           == okapy::PatchResult::InvalidId);
    assert(manager.add(makePatch("example.patch", "Bad.Owner", 0x1000))
           == okapy::PatchResult::InvalidOwner);
    assert(manager.add(makePatch("example.patch", "example.mod", 0))
           == okapy::PatchResult::InvalidAddress);
    assert(manager.add(makePatch("example.patch", "example.mod", 0x1000, {}, {}))
           == okapy::PatchResult::EmptyPatch);
    assert(manager.add(makePatch("example.patch", "example.mod", 0x1000,
                                 {0x01}, {0x02, 0x03}))
           == okapy::PatchResult::SizeMismatch);
    assert(manager.add(makePatch("example.patch", "example.mod",
                                 std::numeric_limits<std::uintptr_t>::max()))
           == okapy::PatchResult::AddressOverflow);
}

void testDuplicateAndOverlapDetection() {
    okapy::PatchManager manager;
    assert(manager.add(makePatch("example.first", "example.mod", 0x1000))
           == okapy::PatchResult::Added);
    assert(manager.add(makePatch("example.first", "other.mod", 0x2000))
           == okapy::PatchResult::DuplicateId);
    assert(manager.add(makePatch("example.overlap", "other.mod", 0x1001))
           == okapy::PatchResult::Overlap);
    assert(manager.add(makePatch("example.adjacent", "other.mod", 0x1002))
           == okapy::PatchResult::Added);
}

void testStateAndOwnerDisable() {
    okapy::PatchManager manager;
    manager.add(makePatch("mod.one", "example.mod", 0x1000));
    manager.add(makePatch("mod.two", "example.mod", 0x2000));
    manager.add(makePatch("other.one", "other.mod", 0x3000));

    assert(manager.setEnabled("mod.one", true));
    assert(manager.setEnabled("mod.two", true));
    assert(manager.setEnabled("other.one", true));
    assert(manager.disableOwner("example.mod") == 2);
    assert(manager.find("mod.one")->state == okapy::PatchState::Disabled);
    assert(manager.find("other.one")->state == okapy::PatchState::Enabled);
    assert(!manager.setEnabled("missing.patch", true));
}

void testListRemoveAndClear() {
    okapy::PatchManager manager;
    manager.add(makePatch("z.last", "example.mod", 0x1000));
    manager.add(makePatch("a.first", "example.mod", 0x2000));
    const auto patches = manager.list();
    assert(patches.size() == 2);
    assert(patches.front().id == "a.first");
    assert(manager.remove("a.first"));
    assert(!manager.remove("a.first"));
    manager.clear();
    assert(manager.size() == 0);
}

} // namespace

int main() {
    testIdValidation();
    testAddAndFind();
    testRejectInvalidPatches();
    testDuplicateAndOverlapDetection();
    testStateAndOwnerDisable();
    testListRemoveAndClear();
    return 0;
}
