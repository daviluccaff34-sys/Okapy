# Okapy — project file index

**Purpose:** authoritative path inventory for the current `main` branch. Update this document whenever a project file is created, removed, renamed, or moved. Before editing an existing file, fetch its current version from GitHub; do not rely on an old local copy.

**Last inventory check:** 2026-11-16  
**Tracked files at check:** 72

## Repository root

- `.gitignore`
- `CMakeLists.txt`
- `LICENSE`
- `README.md`

## GitHub automation

- `.github/workflows/android-build.yml`

## Core — build and documentation

- `core/CMakeLists.txt`
- `core/README.md`

## Core — public headers

- `core/include/okapy/config_store.hpp`
- `core/include/okapy/core.hpp`
- `core/include/okapy/dependency_manager.hpp`
- `core/include/okapy/event_system.hpp`
- `core/include/okapy/hook_manager.hpp`
- `core/include/okapy/installed_mod_catalog.hpp`
- `core/include/okapy/mod_abi.h`
- `core/include/okapy/mod_abi.hpp`
- `core/include/okapy/mod_lifecycle.hpp`
- `core/include/okapy/mod_loader.hpp`
- `core/include/okapy/mod_registry.hpp`
- `core/include/okapy/native_mod_loader.hpp`
- `core/include/okapy/native_module.hpp`
- `core/include/okapy/patch_manager.hpp`
- `core/include/okapy/permission_manager.hpp`

## Core — implementation

- `core/src/config_store.cpp`
- `core/src/core.cpp`
- `core/src/dependency_manager.cpp`
- `core/src/hook_manager.cpp`
- `core/src/installed_mod_catalog.cpp`
- `core/src/mod_abi.cpp`
- `core/src/mod_lifecycle.cpp`
- `core/src/mod_loader.cpp`
- `core/src/mod_registry.cpp`
- `core/src/native_mod_loader.cpp`
- `core/src/native_module.cpp`
- `core/src/patch_manager.cpp`

## Core — tests and test modules

- `core/tests/abi_test_mod.cpp`
- `core/tests/native_test_module.cpp`
- `core/tests/test_config_store.cpp`
- `core/tests/test_dependency_manager.cpp`
- `core/tests/test_event_api.cpp`
- `core/tests/test_event_system.cpp`
- `core/tests/test_hook_manager.cpp`
- `core/tests/test_installed_mod_catalog.cpp`
- `core/tests/test_mod_abi.cpp`
- `core/tests/test_mod_lifecycle.cpp`
- `core/tests/test_mod_loader.cpp`
- `core/tests/test_mod_registry.cpp`
- `core/tests/test_native_mod_loader.cpp`
- `core/tests/test_native_module.cpp`
- `core/tests/test_patch_manager.cpp`
- `core/tests/test_permission_system.cpp`

## Project documentation

- `docs/DEVTOOLS-STUDY.md`
- `docs/GEODE-RESEARCH.md`
- `docs/OKAPY-ARCHITECTURE.md`
- `docs/OKAPY-PACKAGE-FORMAT.md`
- `docs/PROJECT-FILE-INDEX.md` (this file)
- `docs/ROADMAP.md`

## Launcher documentation

- `launcher/README.md`

## Android project configuration

- `launcher/android/app/build.gradle.kts`
- `launcher/android/app/src/main/AndroidManifest.xml`
- `launcher/android/build.gradle.kts`
- `launcher/android/gradle.properties`
- `launcher/android/settings.gradle.kts`

## Android JNI / C++ bridge

- `launcher/android/app/src/main/cpp/CMakeLists.txt`
- `launcher/android/app/src/main/cpp/native-lib.cpp`

## Android Kotlin application

- `launcher/android/app/src/main/java/com/okapy/launcher/MainActivity.kt`
- `launcher/android/app/src/main/java/com/okapy/launcher/ModLibraryActivity.kt`
- `launcher/android/app/src/main/java/com/okapy/launcher/OkapyNative.kt`

## Android package management

- `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyModManager.kt`
- `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyPackageInstaller.kt`
- `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyPackageValidator.kt`

## Android resources

- `launcher/android/app/src/main/res/values/strings.xml`
- `launcher/android/app/src/main/res/values/styles.xml`

## Product requirement — mod management happens in-game

The primary user-facing place to discover, install, update, enable, disable, and remove mods is the **Okapy interface rendered inside Geometry Dash**. This interface must be part of the game's Cocos2d-x scene graph and lifecycle. It is not an Android Activity, WebView, floating overlay, or separate launcher screen.

The Android `ModLibraryActivity` and Kotlin package manager currently form an early package-management/development path. They must not become the final primary mod-management experience. The eventual in-game UI will call the appropriate native/runtime services for repository browsing, package download and validation, installation, activation state, disabling, and removal.

Disabling a running native mod may initially take effect on the next game launch if safe live unloading is not yet supported. The in-game interface must clearly explain that behavior. Removal must safely stop or disable the mod before deleting its files.

## File-change discipline

1. Treat this index as the project-wide file map, not as a substitute for reading source.
2. Fetch the current GitHub file before modifying it and use its current blob SHA.
3. Keep existing architecture and package-format documents synchronized with implementation.
4. When adding or moving files, update this index in the same work sequence.
5. Never recreate an existing file under a guessed path or overwrite newer work with stale content.
6. Do not claim a build or test passed unless it was actually run.
