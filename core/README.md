# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- Mod metadata record: ID, name, version, developer, package path and state.
- ID validation (lowercase letters, digits, underscore, hyphen and at most one dot).
- Duplicate-ID rejection, lookup, state changes, removal and sorted listing.

## Not implemented yet

- Reading or extracting `.okapy` archives.
- Dependency resolution.
- Loading native mod libraries or applying hooks/patches.
- Geometry Dash/Cocos2d-x integration.
- JNI bridge.

The registry stores metadata only. It does not execute mod code or modify game memory.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```
