# Geode research notes

Geode is a Geometry Dash mod loader and SDK, not merely an installer. Its ecosystem separates the core C++ loader, platform launchers, SDK/CLI, documentation, and mod distribution.

## Architecture observations

- The core loader initializes logging, safe-mode behavior, its internal mod, loader services, crash handling, IPC, and optional update checks.
- Mod implementation tracks metadata, platform information, hooks, patches, load state, dependencies/dependants, settings, saved data, load problems, and native binaries.
- The Android launcher is a separate Android application. Its documented responsibilities include preparing and launching a vanilla Geometry Dash installation with the loader integrated. It also documents recovery entry points for safe mode, settings, logs, and crash reports.

## Lessons for Okapy

1. Keep the launcher separate from the in-game runtime.
2. Model launch and mod-loading lifecycle states explicitly.
3. Make safe mode, logs, and recovery first-class features.
4. Separate metadata, dependency resolution, and binary loading.
5. Isolate platform-specific behavior behind platform adapters.

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
- https://github.com/geode-sdk/docs
