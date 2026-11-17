# Okapy — project file index

**Purpose:** authoritative path inventory for the current `main` branch. Update this document whenever a project file is created, removed, renamed, or moved. Before editing an existing file, fetch its current version from GitHub; do not rely on an old local copy.

**Last inventory check:** 2026-11-17  
**Tracked files at check:** 78

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
- `core/include/okapy/installed_package_store.hpp`
- `core/include/okapy/mod_abi.h`
- `core/include/okapy/mod_abi.hpp`
- `core/include/okapy/mod_lifecycle.hpp`
- `core/include/okapy/mod_loader.hpp`
- `core/include/okapy/mod_registry.hpp`
- `core/include/okapy/native_mod_loader.hpp`
- `core/include/okapy/native_module.hpp`
- `core/include/okapy/package_manifest.hpp`
- `core/include/okapy/patch_manager.hpp`
- `core/include/okapy/permission_manager.hpp`

## Core — implementation

- `core/src/config_store.cpp`
- `core/src/core.cpp`
- `core/src/dependency_manager.cpp`
- `core/src/hook_manager.cpp`
- `core/src/installed_mod_catalog.cpp`
- `core/src/installed_package_store.cpp`
- `core/src/mod_abi.cpp`
- `core/src/mod_lifecycle.cpp`
- `core/src/mod_loader.cpp`
- `core/src/mod_registry.cpp`
- `core/src/native_mod_loader.cpp`
- `core/src/native_module.cpp`
- `core/src/package_manifest.cpp`
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
- `core/tests/test_installed_package_store.cpp`
- `core/tests/test_mod_abi.cpp`
- `core/tests/test_mod_lifecycle.cpp`
- `core/tests/test_mod_loader.cpp`
- `core/tests/test_mod_registry.cpp`
- `core/tests/test_native_mod_loader.cpp`
- `core/tests/test_native_module.cpp`
- `core/tests/test_package_manifest.cpp`
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
