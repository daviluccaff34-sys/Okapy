# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Thread-safe in-memory `ModRegistry` and dependency validation/order planning.
- Persistent `ConfigStore` with per-mod namespaces, bounded binary files and Host API callbacks.
- `NativeModule` for explicit-path shared-library loading and symbol lookup.
- Versioned C-compatible `Okapy Mod ABI v1` structures and validation, including host logging, game information, per-mod configuration and event service callbacks.
- `NativeModLoader` to resolve the ABI entry point, validate metadata and register native callbacks with the lifecycle manager.
- `ModLifecycleManager` for ordered initialization, reverse-order shutdown and failure rollback.
- Typed `EventBus`, metadata-only `HookManager` and metadata-only `PatchManager`.
- Unit tests for core subsystems and native-loader integration.

## NativeModLoader

Include `<okapy/native_mod_loader.hpp>`. Supply a trusted absolute library path, a version-1 host API table and the lifecycle manager:

```cpp
okapy::Core core;
const auto host = core.hostApi();
const auto result = core.nativeMods().load(absoluteLibraryPath, core.lifecycle(), host);
```

The loader opens the library, resolves `OKAPY_MOD_QUERY_SYMBOL`, calls the query entry point, validates ABI structures and mod metadata, then registers initialization and shutdown callbacks. It keeps the library loaded while callbacks may run. `Core` declares its members so lifecycle shutdown occurs before native libraries are released.

This is an explicit-path native loader, not a `.okapy` package installer. It does not discover packages, extract archives, verify signatures or hashes, or decide which mods should be enabled. The caller must validate package provenance and trust before loading.

## Persistent configuration

Include `<okapy/config_store.hpp>` or access the store through `Core::config()`. Select a writable application-owned directory before loading mods:

```cpp
okapy::Core core;
core.config().setStorageDirectory(appDataDirectory / "mod-config");
const auto host = core.hostApi();
core.nativeMods().load(libraryPath, core.lifecycle(), host);
```

The Core-provided Host API implements `config_get` and `config_set`; both operate through the Core-owned `ConfigStore`. Each mod ID gets its own `.okcfg` file. Values are UTF-8 strings; keys are limited to 256 bytes, values to 64 KiB, and each mod can store up to 4096 entries. The binary format is versioned. Writes use a temporary file followed by rename, and failed writes restore the previous in-memory value. This is persistence and namespace separation, not encryption: mods execute in-process and must be treated as trusted code.

## Okapy Mod ABI v1 and Host API

Mod authors include `<okapy/mod_abi.h>` and export `okapy_mod_query`. The query function receives the requested ABI version and host API, returns a versioned callback table and fills mod metadata. The same host API pointer is supplied to the mod's `initialize` callback.

Host API v1 defines optional C-compatible services:

- `config_get` / `config_set`: read and write UTF-8 string values in the namespace named by the supplied mod ID.
- `event_subscribe`: subscribe to a named event and receive a nonzero subscription token.
- `event_unsubscribe`: cancel a previous subscription using its token.
- `event_publish`: synchronously publish a named event with a UTF-8 string payload and optionally receive the number of listeners invoked.
- `log` and `get_game_info`: reserved optional service slots; the Core-provided table currently leaves them null.

Event names are case-sensitive UTF-8 strings. A payload is an opaque UTF-8 string; JSON is a convenient convention, but the Core does not parse or validate it. Event callbacks execute synchronously on the thread that publishes the event. The event name and payload pointers are borrowed only for the callback duration and must not be retained. A mod must unsubscribe before its callback function or user data becomes invalid. Callbacks should be short and must not assume delivery on the game's main thread. There is no cross-thread queue, event filtering by priority, or automatic owner-based subscription cleanup yet.

The ABI uses fixed-width integers, opaque context pointers and C function pointers. It avoids passing C++ standard-library objects or exceptions across the shared-library boundary. `struct_size` supports compatible extension; the ABI version identifies incompatible revisions. Host service callbacks are optional and must be checked for null before use. The returned API table and metadata strings must remain valid until the library is unloaded.

The host-side `validateModAbi` checks required structure sizes, version values, metadata pointers and the initializer callback. ABI validation is a compatibility check, not a security mechanism or signature verification.

**Security:** loading or querying a native library executes its code inside the Okapy/Geometry Dash process with that process's permissions. Native mods are not sandboxed. Only load libraries from sources you trust.

## ModLifecycleManager

Modules initialize in dependency-first order and shut down in reverse order. If initialization fails, the failing module's shutdown callback is attempted, followed by rollback of previously started modules. Callback exceptions are caught and recorded. The lifecycle manager is intended for serialized calls from the host's main/control thread; it is not thread-safe.

## Other Core systems

- `ModLoader` prepares dependency-safe order from registered metadata; it does not execute code.
- `NativeModule` opens a shared library from an absolute path. An absolute path is not a trust check.
- `EventBus` dispatches typed events synchronously on the publishing thread.
- `HookManager` stores hook metadata only; no detour is installed.
- `PatchManager` stores patch metadata only; it does not read or write process memory.

## Not implemented yet

- Reading, validating, or extracting `.okapy` archives.
- Automatic package discovery, integrity verification and enable/disable policy.
- Automatic event-subscription cleanup when a mod stops/unloads and a cross-thread event queue.
- A stable public mod SDK.
- Geometry Dash/Cocos2d-x integration and JNI bridge.
- Actual hook installation or memory patch application.

Dependency versions currently use strict `MAJOR.MINOR.PATCH` syntax; prerelease/build metadata and version ranges are not supported yet.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
