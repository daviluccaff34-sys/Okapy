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
- Numeric `MAJOR.MINOR.PATCH` minimum-version checks (inclusive).
- Mod metadata record: ID, name, version, developer, package path and state.
- Unit tests for registry operations, dependency resolution, event dispatch and hook metadata management.

## EventSystem

Include `<okapy/event_system.hpp>` and define an event as a normal C++ type:

```cpp
struct PlayerJoined {
    int playerId;
};

okapy::EventBus events;
auto listener = events.subscribe<PlayerJoined>(
    [](const PlayerJoined& event) {
        // React to the event.
    },
    10 // Higher priority runs first.
);

events.publish(PlayerJoined{42});
```

Subscriptions return move-only `ListenerHandle` objects. Destroying or resetting
a handle unsubscribes it. Dispatch uses a listener snapshot, so callbacks may
subscribe or unsubscribe without holding the bus mutex. Listeners are called
synchronously on the thread that publishes the event; callback exceptions
propagate to the publisher.

## HookManager

Include `<okapy/hook_manager.hpp>` to register hook descriptions:

```cpp
okapy::HookManager hooks;
hooks.add({"sample.update", "sample.mod", 0x1000, 0x2000});
hooks.setEnabled("sample.update", true);
```

The manager validates IDs and addresses, rejects duplicate hook IDs and
multiple registrations for the same target, tracks the owning mod, and can
disable all logically enabled hooks belonging to one owner. Its state is
metadata only: `setEnabled` does not install a detour or modify executable
memory. Real platform-specific hook backends are a later integration task.

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

The EventBus dispatches typed C++ events but does not execute mod code by itself.
The HookManager tracks hook descriptions and requested states, but does not
redirect function calls, patch bytes or modify game memory.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
