# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- `DependencyManager` for enabled-mod dependency validation, conflict detection, cycle detection and deterministic dependency-first ordering.
- Typed `EventBus` with RAII listener handles, priorities, per-event clearing and snapshot-based dispatch.
- `HookManager` for hook metadata registration, owner tracking, target conflict checks and logical state management.
- `PatchManager` for patch metadata, expected/replacement byte validation, address overflow checks, overlap detection and owner-based state management.
- Unit tests for registry operations, dependency resolution, event dispatch, hook management and patch management.

## EventSystem

Include `<okapy/event_system.hpp>` and define an event as a normal C++ type:

```cpp
struct PlayerJoined { int playerId; };

okapy::EventBus events;
auto listener = events.subscribe<PlayerJoined>(
    [](const PlayerJoined& event) { /* React to event. */ }, 10);
events.publish(PlayerJoined{42});
```

Subscriptions return move-only `ListenerHandle` objects. Destroying or resetting
a handle unsubscribes it. Dispatch uses a listener snapshot; callbacks execute
synchronously on the publishing thread.

## HookManager

Include `<okapy/hook_manager.hpp>` to register hook descriptions:

```cpp
okapy::HookManager hooks;
hooks.add({"sample.update", "sample.mod", 0x1000, 0x2000});
hooks.setEnabled("sample.update", true);
```

The manager validates IDs and addresses, rejects duplicate IDs and multiple
registrations for the same target, tracks ownership and can disable all hooks
belonging to one owner. State changes are metadata only; no detour is installed.

## PatchManager

Include `<okapy/patch_manager.hpp>` to register patch descriptions:

```cpp
okapy::PatchManager patches;
patches.add({
    "sample.change", "sample.mod", 0x3000,
    {0x10, 0x20}, {0x30, 0x40}
});
patches.setEnabled("sample.change", true);
```

Each patch requires a non-zero target, non-empty expected and replacement byte
sequences of equal length, and an address range that does not overflow.
Overlapping target ranges and duplicate IDs are rejected. Patches are associated
with an owner and can be disabled by owner.

**Safety boundary:** PatchManager stores and validates descriptions only.
It does not read or write process memory, change executable pages, or apply
patches. `setEnabled` changes only the recorded state. A platform-specific,
permission-aware patch backend is a separate future component.

## Not implemented yet

- Reading or extracting `.okapy` archives.
- Loading native mod libraries or applying hooks/patches.
- Geometry Dash/Cocos2d-x integration.
- JNI bridge.
- A stable public mod SDK or cross-thread event queue.

The registry and dependency manager store and analyze metadata only. The
dependency manager reports missing required dependencies, incompatible minimum
versions, declared conflicts, malformed versions, duplicate manifests and
dependency cycles. A load order is returned only when the graph has no reported
errors. Missing optional dependencies are ignored. Versions currently use strict
`MAJOR.MINOR.PATCH` syntax; prerelease/build metadata and version ranges are
not supported yet.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
