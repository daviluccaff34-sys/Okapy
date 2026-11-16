# Okapy architecture

**Status:** Initial design, not a stable implementation contract.

Okapy is an independent modding platform for Geometry Dash. Its architecture is informed by studying public Geode documentation and behavior, but its source code, APIs, package rules, internal names, and implementation are designed separately. No Geode source files, code snippets, assets, or private implementation details are used as Okapy implementation material.

## Design goals

- Keep the Android launcher, native runtime, and developer SDK as separate components.
- Inspect compatibility before installation or execution.
- Treat native mods as executable code and require explicit user consent before enabling them.
- Make startup failures recoverable through safe mode and diagnostic logs.
- Keep game-version and Android-ABI-specific behavior behind narrow interfaces.
- Make lifecycle states and dependency decisions explicit.
- Do not claim compatibility until tested on a documented game version and device.

## Components

### 1. Android launcher

Responsibilities:
- Detect the supported Geometry Dash installation.
- Present installed mods, package information, compatibility warnings, and user controls.
- Import .okapy packages through Android's document picker.
- Validate packages before copying them into Okapy's private storage.
- Maintain enabled/disabled selections and launch preferences.
- Provide normal launch, safe launch, diagnostics, and recovery entry points.

The current launcher is a basic Android Activity that detects and opens the game through Android's package manager. It does not host the game process, integrate native libraries, or load Okapy Core. An initial extraction installer exists but is not connected to the launcher UI.

### 2. Okapy Core

A future native C++ runtime, with a small JNI boundary to the Android application.

Proposed internal services:
- **RuntimeController:** coordinates startup and shutdown states.
- **PackageCatalog:** reads installed package metadata without executing mod code.
- **DependencyPlanner:** checks required dependencies, version constraints, and conflicts before activation.
- **ModLifecycle:** controls discovery, preparation, activation, failure, and deactivation.
- **NativeModuleBackend:** platform-specific native-library loading behind an interface.
- **HookBackend:** a replaceable integration layer; no hook implementation is selected yet.
- **ResourceService:** exposes mod-owned resources through controlled paths.
- **ConfigService:** stores per-mod settings and persistent data.
- **DiagnosticService:** records startup phases, mod errors, and crash context.
- **RecoveryPolicy:** starts without third-party mods when safe mode is requested.

These are design boundaries, not implemented classes yet. Their names and contracts may change as the runtime is prototyped.

### 3. Okapy SDK

A separate developer-facing API and build toolchain. The SDK must define:
- Stable mod metadata and lifecycle contracts.
- Supported game APIs and compatibility declarations.
- Logging, configuration, and resource access.
- Dependency declarations and version constraints.
- A reproducible package-building command.
- Header/API versioning and examples.

The SDK must not expose unstable Core internals as public API.

### 4. .okapy package

The current Draft 0.1 is a ZIP-based package with a root okapy.json manifest and Android ABI-specific native entrypoints. The format is not frozen. Before a stable release, it needs strict JSON rules, dependency constraints, integrity hashes, signing/trust decisions, and explicit compatibility semantics.

A valid manifest is not proof that a native library is safe. Validation, installation, trust decisions, and execution are separate operations.

## Proposed runtime lifecycle

1. **Bootstrap:** the launcher selects normal or safe mode and passes a validated launch configuration.
2. **Environment check:** Core verifies supported game version, ABI, storage paths, and runtime prerequisites.
3. **Catalog:** installed package metadata is read without loading native code.
4. **Planning:** dependencies, conflicts, enabled state, and compatibility are evaluated as a complete plan.
5. **Preparation:** required files and resources are checked and runtime services are initialized.
6. **Activation:** eligible native modules are loaded in dependency order; each activation is tracked independently.
7. **Running:** runtime events and diagnostics are delivered through documented interfaces.
8. **Failure recovery:** a failed mod is isolated where possible; the next launch can use safe mode to skip third-party modules.
9. **Shutdown:** runtime-owned resources and diagnostics are finalized.

No implementation should assume that unloading arbitrary native code is always safe. The first runtime may support disabling a mod for the next launch rather than unloading it live.

## Android integration: unresolved engineering gate

The current launcher only opens the installed game with a normal Android launch Intent. That cannot, by itself, load a native mod runtime into the game's process.

A future Android integration must be researched and prototyped independently. It may require an Android host Activity, JNI, NDK-built libraries, and a compatible way to initialize the game's native engine. This is a major technical gate: do not implement guessed library names, offsets, patches, or version assumptions. Do not modify the game installation until the integration method, compatibility constraints, and recovery path are understood and tested.

## Security boundaries

- Never extract archive entries using untrusted paths without canonical-path containment checks.
- Enforce compressed and expanded size limits while reading actual bytes.
- Reject duplicate normalized paths and malformed manifests.
- Do not load a package during validation or preview.
- Clearly identify that native mods can execute code with the app's available privileges.
- Require user confirmation before installing or enabling third-party native code.
- Keep safe mode independent from third-party mod state.
- Do not call a mod trusted solely because its package signature or manifest is valid.

## Independence and licensing

Okapy uses Geode as a public technical reference for understanding the problem space and user-facing capabilities. Okapy implementation must be written independently. Do not copy Geode code, assets, SDK declarations, internal names, or package format. When a behavior is useful, document the requirement in general terms and design an Okapy-specific interface and implementation. Keep a record of third-party dependencies and comply with their licenses.

## Current implementation status

- Android launcher foundation, game detection, and ordinary launch Intent: implemented; physical-device verification remains.
- .okapy ZIP/manifest validator: initial implementation exists; not yet built or tested.
- .okapy extraction installer: initial implementation exists; not yet built or tested and not yet connected to the UI.
- Native Core, JNI bridge, dependency planner, SDK, hook backend, and native loading: not implemented.
- Android game-host integration: not selected or implemented.
