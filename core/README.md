# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- `DependencyManager` for enabled-mod dependency validation, conflict detection, cycle detection and deterministic dependency-first ordering.
- Numeric `MAJOR.MINOR.PATCH` minimum-version checks (inclusive).
- Mod metadata record: ID, name, version, developer, package path and state.
- ID validation (lowercase letters, digits, underscore, hyphen and at most one dot).
- Duplicate-ID rejection, lookup, state changes, removal and sorted listing.
- Unit tests for ID validation, registry operations and dependency resolution.

## Not implemented yet

- Reading or extracting `.okapy` archives.
- Loading native mod libraries or applying hooks/patches.
- Geometry Dash/Cocos2d-x integration.
- JNI bridge.

The registry and dependency manager store and analyze metadata only. The dependency manager reports missing required dependencies, incompatible minimum versions, declared conflicts, malformed versions, duplicate manifests and dependency cycles. A load order is returned only when the graph has no reported errors. Missing optional dependencies are ignored. Versions currently use strict `MAJOR.MINOR.PATCH` syntax; prerelease/build metadata and version ranges are not supported yet.

Neither component executes mod code, loads native libraries or modifies game memory.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
