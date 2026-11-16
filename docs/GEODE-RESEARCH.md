# Geode research notes

Geode is a Geometry Dash mod loader and SDK, not merely an installer. Its public ecosystem separates the core loader, platform launchers, SDK/CLI, documentation, and mod distribution.

These notes record high-level observations from public documentation and source files. They are used to understand engineering requirements, not as source material to copy into Okapy.

## Core loader observations

The public loader implementation shows several distinct responsibilities:

- Startup creates and prepares runtime directories for saves, resources, configuration, mods, logs, temporary files, and runtime data.
- Mod discovery scans configured search directories for .geode packages and builds metadata records before loading native binaries.
- Mod instances track metadata, platform information, hooks, patches, load state, dependencies and dependants, settings, saved data, resource state, logs, and load problems.
- Dependency relationships are refreshed as a graph rather than treated as a simple list.
- Native loading is a separate phase from metadata discovery.
- In safe mode, third-party native code is skipped while mods can remain visible to the user for recovery and management.
- Hook and patch ownership is tracked per mod so the loader can disable or clean up resources associated with a failed module.
- Enable/disable requests are persisted as desired state; they are not necessarily equivalent to unloading arbitrary native code immediately.

These observations support a general design principle for Okapy: package discovery, dependency planning, native activation, and recovery must be separate phases.

## Android launcher observations

The public Android launcher source demonstrates that its game-launching path is substantially more complex than calling Android's normal launch Intent.

At a high level, its game-host Activity:

1. Checks that the Geometry Dash package is installed and reads its package/version information.
2. Prepares asset and save-path redirection.
3. Initializes the Cocos2d Android helper and connects Android-side utilities to native code.
4. Loads the game's FMOD and Cocos native libraries from the installed game package, accounting for extracted libraries and split APK layouts.
5. Applies version-dependent launcher fixes.
6. Initializes internal native components before creating the Cocos OpenGL surface and renderer.
7. Performs post-library initialization and then loads the Geode native library.
8. Catches native linkage and startup errors and returns to the launcher with diagnostic information.

The launcher also provides recovery-oriented navigation for safe mode, settings, logs, and crash reports. These are important operational features because a mod loader can otherwise make the game difficult to start after a broken mod or incompatible update.

This is an architectural observation only. Okapy must not copy this Activity, its code, patches, offsets, or assets. Android integration must be independently designed and tested for the exact game versions Okapy supports.

## Dependency and mod lifecycle observations

Geode's public dependency documentation describes required and optional dependencies and version constraints. Its loader implementation maintains relationships between mods and uses a dependency graph to coordinate loading.

The useful general lesson is that Okapy should not activate mods in arbitrary file order. It needs a deterministic planner that can:
- detect missing required dependencies;
- distinguish optional dependencies from required ones;
- evaluate version constraints;
- detect cycles and conflicts;
- produce a stable activation order;
- explain why a mod was skipped.

Okapy's current .okapy draft only has basic dependency ID validation. It does not yet implement version constraints or dependency planning.

## Lessons for Okapy

1. Keep the Android launcher separate from the native runtime.
2. Model launch and mod-loading lifecycle states explicitly.
3. Make safe mode, logs, and recovery first-class features.
4. Separate package metadata, dependency resolution, and native loading.
5. Track ownership of runtime resources so failed mods can be isolated.
6. Keep platform-specific behavior behind adapters.
7. Treat package validation as structural inspection, not proof that native code is safe.
8. Do not assume arbitrary native modules can be unloaded safely at runtime.
9. Do not claim compatibility until it has been tested on a documented device and game version.

## Current Okapy implementation status

- The Android launcher currently detects Geometry Dash and opens it using Android's ordinary package launch Intent. It does not host the game or load native libraries.
- The .okapy validator reads ZIP entries and counts actual expanded bytes rather than trusting ZIP size metadata alone. It has not yet been built or tested.
- The validator does not install files, resolve dependencies, load native code, or guarantee that a mod will not crash the game.
- The native Core, JNI bridge, dependency planner, installer, SDK, and native loading are not implemented.
- Android game-host integration remains an engineering investigation and must not be guessed.

## Independence and licensing

Okapy uses Geode as a public technical reference for understanding the problem space and the capabilities a mature mod loader may need. Okapy's code, APIs, package format, names, assets, and implementation must be independently designed and written. Do not copy Geode source files or code snippets. Track all third-party dependencies and comply with their licenses.

## Public references

- [Geode loader repository](https://github.com/geode-sdk/geode)
- [Geode Android launcher](https://github.com/geode-sdk/android-launcher)
- [Geode documentation](https://github.com/geode-sdk/docs)
- [Geode dependency documentation](https://github.com/geode-sdk/docs/blob/main/mods/dependencies.md)
