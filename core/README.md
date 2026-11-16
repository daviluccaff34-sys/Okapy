# Okapy Core

The native foundation of Okapy is written in C++17. It is deliberately kept
independent of Android and Geometry Dash so core systems can be developed and
tested without loading the game.

## Current status

- Initial CMake target: `Okapy::Core`.
- Public core identity and version constants.
- Minimal initialization state.

## Planned modules

- Package metadata and secure `.okapy` archive inspection.
- Installed mod registry.
- Dependency and incompatibility resolution.
- Event dispatch system.
- Runtime lifecycle and native integration.
- JNI bridge to the Android launcher.

No mod binaries are loaded and no Geometry Dash memory is modified by this
initial foundation.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```
