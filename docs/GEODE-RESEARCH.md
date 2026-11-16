# Geode research notes

Geode is a Geometry Dash mod loader and SDK, not merely an installer. Its ecosystem separates the core C++ loader, platform launchers, SDK/CLI, documentation, and mod distribution.

## Architecture observations

- The core loader initializes logging, safe-mode behavior, its internal mod, loader services, crash handling, IPC, and optional update checks.
- Mod implementation tracks metadata, platform information, hooks, patches, load state, dependencies/dependants, settings, saved data, load problems, and native binaries.
- The Android launcher is a separate Android application. Its documented responsibilities include preparing and launching a vanilla Geometry Dash installation with the loader integrated. It also exposes deep-link recovery entry points for the main screen, explicit launch, safe mode, settings, developer settings, logs, and crash reports. These are deliberate escape hatches when normal startup is impaired. [Android launcher README](https://github.com/geode-sdk/android-launcher).

## Lessons for Okapy

1. Keep the launcher separate from the in-game runtime.
2. Model launch and mod-loading lifecycle states explicitly.
3. Make safe mode, logs, and recovery first-class features.
4. Separate metadata, dependency resolution, and binary loading.
5. Isolate platform-specific behavior behind platform adapters.\n6. Treat safe-mode launch, logs, and crash reports as operational recovery features, not optional decoration.\n7. Keep package validation separate from runtime loading: validating a ZIP does not prove that its native library is safe or compatible.\n\n## Current study applied to Okapy\n\n- The current `.okapy` validator now counts bytes actually read from ZIP entries, enforcing the total expanded-size limit even if ZIP size metadata is missing or misleading. The manifest is captured with its own 64 KiB read limit.\n- This remains structural validation only. It does not install files, resolve dependencies, load native code, or guarantee that a mod will not crash the game.\n- Future launcher design should reserve explicit flows for normal launch, safe mode, settings, diagnostics/logs, and crash recovery, inspired by the documented Android launcher escape hatches but implemented independently.

Okapy will use its own implementation, code, names, metadata format, and APIs. This is a high-level architectural study, not a copy of Geode source code or assets.

## Decisions still required

- First supported platform (Android, Windows, or another platform).
- Supported Geometry Dash versions and installation sources.
- Platform-appropriate game integration method.
- Whether v0.1.0 only launches the game or also loads `.okapy` mods.
- Initial `.okapy` package metadata schema.

## References

- https://github.com/geode-sdk/geode
- https://github.com/geode-sdk/android-launcher
- https://github.com/geode-sdk/docs\n- https://github.com/geode-sdk/docs/blob/main/mods/dependencies.md
