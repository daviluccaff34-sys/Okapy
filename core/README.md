# Okapy Core

The native foundation of Okapy is written in C++17 and kept independent from
Android and Geometry Dash so its systems can be developed and tested separately.

## Implemented

- CMake target: `Okapy::Core`.
- Core identity/version constants and initialization state.
- Thread-safe in-memory `ModRegistry`.
- `DependencyManager` for enabled-mod dependency validation, conflict detection, cycle detection and deterministic dependency-first ordering.
- `ModLoader` to prepare metadata-only load plans using dependency validation and ordering.
- `NativeModule` for explicit-path shared-library loading, symbol lookup, RAII cleanup and platform error reporting.
- `ModLifecycleManager` for ordered initialization, reverse-order shutdown, failure rollback and lifecycle status reporting.
- Versioned C-compatible `Okapy Mod ABI v1` structures and ABI layout/callback validation.
- Typed `EventBus` with RAII listener handles, priorities, per-event clearing and snapshot-based dispatch.
- `HookManager` for hook metadata registration, owner tracking, target conflict checks and logical state management.
- `PatchManager` for patch metadata, expected/replacement byte validation, address overflow checks, overlap detection and owner-based state management.
- Unit tests for registry operations, dependency resolution, event dispatch, hook management, patch management, load-plan preparation, native module loading, lifecycle behavior and ABI validation.

## Okapy Mod ABI v1

Mod authors include the C-compatible header `<okapy/mod_abi.h>` and export
`okapy_mod_query`. The query function receives the requested ABI version and
the host API, then returns a versioned table of callbacks and fills metadata:

```cpp
#define OKAPY_MOD_BUILD
#include <okapy/mod_abi.h>

static int32_t mod_initialize(
    void* context, const OkapyHostApiV1* host) {
    if (host && host->log) {
        host->log(host->host_context, 1, "Hello from my mod");
    }
    return 1;
}

static void mod_shutdown(void* context) {
    // Release mod-owned resources.
}

static const OkapyModApiV1 api = {
    sizeof(OkapyModApiV1),
    OKAPY_MOD_ABI_VERSION_1,
    nullptr,
    mod_initialize,
    mod_shutdown
};

extern "C" OKAPY_MOD_EXPORT const OkapyModApiV1* okapy_mod_query(
    uint32_t requested_abi,
    const OkapyHostApiV1* host,
    OkapyModInfoV1* out_info) {
    if (requested_abi != OKAPY_MOD_ABI_VERSION_1 || !out_info) {
        return nullptr;
    }
    *out_info = {
        sizeof(OkapyModInfoV1),
        OKAPY_MOD_ABI_VERSION_1,
        "author.example",
        "Example Mod",
        "1.0.0"
    };
    return &api;
}
```

The ABI uses fixed-width integer types, opaque context pointers and C function
pointers. It deliberately avoids passing C++ standard-library objects or
exceptions across the shared-library boundary. `struct_size` allows future
compatible extension; the ABI version identifies incompatible revisions.
The returned API table and metadata strings must remain valid until the module
is unloaded. A mod must not throw exceptions through ABI callbacks.

The host-side `validateModAbi` helper checks required structure sizes,
version values, metadata pointers and the required initializer callback. It
does not call the query function or prove that a library is safe. The host must
load only trusted libraries, resolve `OKAPY_MOD_QUERY_SYMBOL`, call the query
function, validate its result, and keep the library loaded until shutdown has
finished.

**Security boundary:** native mods execute with the process's permissions and
are not sandboxed. ABI validation is a compatibility check, not a security
mechanism or signature verification. Do not load untrusted native code.

## ModLifecycleManager

Include `<okapy/mod_lifecycle.hpp>`. The host registers callbacks only after
validating a mod package and its ABI, then starts modules using a valid plan.
Modules initialize in dependency-first order and shut down in reverse order.
If initialization fails, the failing module's shutdown callback is attempted,
followed by rollback of previously started modules. Callback exceptions are
caught and recorded. The manager also shuts down remaining running modules
when it is destroyed.

The lifecycle manager is intended for serialized calls from the host's
main/control thread; it is not thread-safe. The host must ensure callback code
and its native library remain valid until shutdown completes.

## NativeModule

Include `<okapy/native_module.hpp>` to open a native shared library and resolve
an exported symbol. The loader uses `LoadLibraryW` on Windows and `dlopen` on
Android/Linux and other POSIX platforms. It requires an absolute path, reports
loading errors, and releases the library when the object is destroyed. The
module must remain loaded for as long as any function pointer obtained from it
is used. An absolute path is not a trust or integrity check.

## ModLoader

Include `<okapy/mod_loader.hpp>` to prepare a dependency-safe order for enabled
mods after their metadata and manifests have been registered. The returned
plan contains a dependency report and deterministic dependency-first mod IDs.
Missing required dependencies, version mismatches, declared conflicts,
invalid versions, duplicate manifests and dependency cycles prevent the plan
from being ready. An empty plan is not considered ready. ModLoader does not
itself execute code.

## EventSystem

Include `<okapy/event_system.hpp>`. Subscriptions return move-only
`ListenerHandle` objects. Destroying or resetting a handle unsubscribes it.
Dispatch uses a listener snapshot; callbacks execute synchronously on the
publishing thread.

## HookManager

Include `<okapy/hook_manager.hpp>` to register hook descriptions. It validates
IDs and addresses, rejects duplicate IDs and multiple registrations for the
same target, tracks ownership and can disable all hooks belonging to one owner.
State changes are metadata only; no detour is installed.

## PatchManager

Include `<okapy/patch_manager.hpp>` to register patch descriptions. Each patch
requires a non-zero target, non-empty expected and replacement byte sequences
of equal length, and an address range that does not overflow. Overlapping
target ranges and duplicate IDs are rejected. It stores metadata only and does
not read or write process memory.

## Not implemented yet

- Reading, validating signatures for, or extracting `.okapy` archives.
- Automatic connection between package metadata, NativeModule, ABI query and lifecycle.
- A stable public mod SDK and cross-thread event queue.
- Geometry Dash/Cocos2d-x integration and JNI bridge.
- Actual hook installation or memory patch application.

The registry and dependency manager store and analyze metadata only. Dependency
versions currently use strict `MAJOR.MINOR.PATCH` syntax; prerelease/build
metadata and version ranges are not supported yet.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
