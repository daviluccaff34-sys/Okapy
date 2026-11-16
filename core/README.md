# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Thread-safe in-memory `ModRegistry` and dependency validation/order planning.
- Per-mod `PermissionManager` with deny-by-default grants for configuration and event services.
- Persistent `ConfigStore` with per-mod namespaces and bounded binary files.
- Versioned C-compatible `Okapy Mod ABI v1` and Host API callbacks for configuration, events and permission checks.
- `NativeModule` and `NativeModLoader` for explicit-path native library loading and lifecycle registration.
- `ModLifecycleManager` for ordered initialization, reverse-order shutdown and failure rollback.
- Typed `EventBus`, metadata-only `HookManager` and metadata-only `PatchManager`.
- Unit tests for core subsystems and native-loader integration.

## Permission System v1

Include `<okapy/permission_manager.hpp>` or access it through `Core::permissions()`. Permissions are deny-by-default and are granted by the host application before the mod uses a protected Host API service.

| Permission | Host API operations |
| --- | --- |
| `config.read` | `config_get` |
| `config.write` | `config_set` |
| `events.subscribe` | `event_subscribe` and subscription ownership |
| `events.publish` | `event_publish` |
| `game.info` | Reserved for the future game-info service |
| `logging.write` | Reserved for the future logging service |

Example host-side policy:

```cpp
okapy::Core core;
core.permissions().grant("example.mod", okapy::kPermissionConfigRead);
core.permissions().grant("example.mod", okapy::kPermissionConfigWrite);
core.permissions().grant("example.mod", okapy::kPermissionEventsSubscribe);
core.permissions().grant("example.mod", okapy::kPermissionEventsPublish);

const auto host = core.hostApi();
```

The C ABI exposes `permission_check(host_context, mod_id, permission)`, returning 1 for granted, 0 for denied and -1 for invalid input. Protected config and event calls check the matching permission and return a permission-denied result when access is not granted. Event subscription tokens are associated with the supplied mod ID; a different ID cannot cancel another mod's token. The host can grant, revoke, list or clear a mod's permissions through the C++ manager.

**Security boundary:** the current ABI passes the mod ID as an argument, so it is an API-level policy mechanism, not an unforgeable identity system. Native mods execute in the same process and can call native code directly or bypass the Host API. This manager is not a sandbox and must not be treated as protection against malicious native code. A future loader revision should bind an unforgeable per-module host context before describing permissions as a security boundary.

## NativeModLoader

Include `<okapy/native_mod_loader.hpp>`. Supply a trusted absolute library path, a version-1 host API table and the lifecycle manager:

```cpp
okapy::Core core;
const auto host = core.hostApi();
const auto result = core.nativeMods().load(absoluteLibraryPath, core.lifecycle(), host);
```

The loader opens the library, resolves `OKAPY_MOD_QUERY_SYMBOL`, validates ABI structures and mod metadata, then registers initialization and shutdown callbacks. It keeps the library loaded while callbacks may run. This is not a `.okapy` package installer and does not verify signatures or hashes. Loading a native library executes its code in-process; only load trusted libraries.

## Persistent configuration

Use `Core::config()` to select an application-owned storage directory before loading mods. Each mod ID gets its own `.okcfg` file. Values are UTF-8 strings; keys are limited to 256 bytes, values to 64 KiB and each mod can store up to 4096 entries. This is persistence and namespace separation, not encryption.

## Event API v1

Event names are case-sensitive UTF-8 strings. Payloads are opaque UTF-8 strings; JSON is a convention, not parsed by Core. Delivery is synchronous on the publishing thread. Callback payload pointers are borrowed only for the callback duration. Mods must unsubscribe before callback code or user data becomes invalid. There is not yet automatic subscription cleanup on mod shutdown or a cross-thread event queue.

## Other Core systems

- `ModLoader` prepares dependency-safe order from metadata; it does not execute code.
- `NativeModule` opens a shared library from an absolute path.
- `HookManager` stores hook metadata only; no detour is installed.
- `PatchManager` stores patch metadata only; it does not read or write process memory.
- Geometry Dash/Cocos2d-x integration and JNI bridge are not implemented yet.

Dependency versions currently use strict `MAJOR.MINOR.PATCH` syntax; prerelease/build metadata and version ranges are not supported yet.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
