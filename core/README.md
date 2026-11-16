# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- `DependencyManager` for enabled-mod dependency validation, conflict detection, cycle detection and deterministic dependency-first ordering.
- Typed `EventBus` with RAII listener handles, priorities, per-event clearing and snapshot-based dispatch.
- Numeric `MAJOR.MINOR.PATCH` minimum-version checks (inclusive).
- Mod metadata record: ID, name, version, developer, package path and state.
- ID validation (lowercase letters, digits, underscore, hyphen and at most one dot).
- Duplicate-ID rejection, lookup, state changes, removal and sorted listing.
- Unit tests for registry operations, dependency resolution and event dispatch.

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
propagate to the publisher. A listener already executing cannot be forcibly
stopped by resetting its handle from another thread.

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

The EventBus dispatches typed C++ events but does not execute mod code by itself,
load native libraries or modify game memory.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
