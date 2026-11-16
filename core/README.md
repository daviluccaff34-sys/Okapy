# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- `DependencyManager` for enabled-mod dependency validation, conflict detection, cycle detection and deterministic dependency-first ordering.
- `ModLoader` to prepare metadata-only load plans using dependency validation and ordering.
- `NativeModule` for explicit-path shared-library loading, symbol lookup, RAII cleanup and platform error reporting.
- Typed `EventBus` with RAII listener handles, priorities, per-event clearing and snapshot-based dispatch.
- `HookManager` for hook metadata registration, owner tracking, target conflict checks and logical state management.
- `PatchManager` for patch metadata, expected/replacement byte validation, address overflow checks, overlap detection and owner-based state management.
- Unit tests for registry operations, dependency resolution, event dispatch, hook management, patch management, load-plan preparation and native module loading.

## NativeModule

Include `<okapy/native_module.hpp>` to open a native shared library and resolve
an exported symbol:

```cpp
okapy::NativeModule module;
const auto result = module.load("/absolute/path/to/mod.so");

if (result == okapy::NativeModuleResult::Loaded) {
    void* entrypoint = module.symbol("okapy_mod_init");
    // Validate the mod ABI before converting/calling the entrypoint.
}
```

The loader uses `LoadLibraryW` on Windows and `dlopen` on Android/Linux and
other POSIX platforms. It requires an absolute path, reports loading errors,
and releases the library when the object is destroyed. The module must remain
loaded for as long as any function pointer obtained from it is used.

**Security warning:** loading a native library executes its code in the
Okapy/Geometry Dash process with that process's permissions. An absolute path
is not a trust or integrity check. Only load mods from sources you trust.
This component does not yet verify signatures, hashes, package manifests or
the Okapy mod ABI, and it does not sandbox native code.

## ModLoader

Include `<okapy/mod_loader.hpp>` to prepare a dependency-safe order for enabled
mods after their metadata and manifests have been registered:

```cpp
okapy::Core core;
core.initialize();

const auto plan = core.loader().prepare(core.mods(), manifests);
if (plan.isReady()) {
    for (const auto& modId : plan.dependencies.loadOrder) {
        // A future lifecycle layer will initialize each mod in this order.
    }
}
```

The returned plan contains the dependency report and a deterministic,
dependency-first list of enabled mod IDs. Missing required dependencies,
version mismatches, declared conflicts, invalid versions, duplicate manifests
and dependency cycles prevent the plan from being considered ready. An empty
plan is not considered ready.

**Boundary:** ModLoader prepares and validates metadata only. It does not
discover files, extract `.okapy` archives, execute entrypoints, or change mod
states. NativeModule is a lower-level library primitive and is not automatically
invoked by ModLoader.

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

**Boundary:** PatchManager stores and validates descriptions only. It does not
read or write process memory, change executable pages, or apply patches.

## Not implemented yet

- Reading, validating signatures for, or extracting `.okapy` archives.
- A stable exported mod ABI and controlled mod lifecycle.
- Automatic native module discovery/loading from a load plan.
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
