# Okapy architecture

**Status:** Initial design, not a stable implementation contract.

Okapy is an independent modding platform for Geometry Dash. Its architecture is informed by studying public Geode documentation and behavior, but its source code, APIs, package rules, internal names, and implementation are designed separately. No Geode source files, code snippets, assets, or private implementation details are used as Okapy implementation material.

## Product experience

Okapy has two user-facing surfaces:

1. **Android launcher:** a deliberately simple home screen inspired by the reference layout supplied by the user: Okapy branding centered on the screen, a prominent Launch button, and a separate settings control. The logo, typography, colors, and assets must be original to Okapy.
2. **In-game Okapy interface:** the main place to discover, install, update, enable, disable, and remove mods. Users should not need to leave Geometry Dash to manage mods.

The launcher is not intended to become a full external mod store. Its primary job is to start the game and provide settings and recovery options. The in-game interface is the main mod-management experience.

## Core technical requirement: direct Cocos2d-x integration

Okapy must integrate directly into the Cocos2d-x runtime used by Geometry Dash. The in-game interface is to be made from native Cocos2d-x scene-graph objects (such as nodes, layers, menus, and event handlers), created and managed inside the game's own process and rendering lifecycle.

This is not an Android overlay, floating window, WebView, or separate screen drawn above the game. It must behave as part of the game UI and participate in the Cocos2d-x scene and event systems.

To achieve this, Okapy will need an independently implemented native integration layer:
- A runtime bootstrap that becomes active in the Geometry Dash process.
- A Cocos2d-x adapter for creating, attaching, updating, and removing Okapy UI nodes on the correct thread and lifecycle.
- A game-integration layer that connects selected game screens and actions to the Okapy interface.
- A hook/patch backend for carefully researched integration points where ordinary Cocos2d-x APIs are insufficient.
- A version/ABI compatibility layer so game-specific integration is not scattered through Core.

The exact Cocos2d-x version, available symbols, binary interfaces, game classes, hook points, and initialization order must be verified for each supported Geometry Dash build. Do not guess class layouts, offsets, symbols, or patch bytes. Do not assume that integrating with Cocos2d-x alone automatically exposes every Geometry Dash-specific feature.

Geode is a public architectural reference for this direct in-process integration approach. Okapy must implement its own bootstrap, Cocos adapter, hook backend, APIs, and UI. No Geode code, assets, SDK declarations, internal names, or package format may be copied.

## Components

### 1. Android launcher

Responsibilities:
- Detect the supported Geometry Dash installation.
- Present the Okapy home screen with Launch and Settings.
- Start the Okapy-enabled game runtime when the Android integration is implemented.
- Provide safe launch, diagnostics, and recovery entry points.

The current launcher is a basic Android Activity that detects and opens the game through Android's package manager. It does not host the game process, integrate native libraries, or load Okapy Core.

### 2. Okapy in-game interface

A native interface rendered inside the running Geometry Dash game through Cocos2d-x.

Planned capabilities:
- Browse available mods from a configured catalog.
- View mod details, version, developer, compatibility, dependencies, and permissions/trust information.
- Download and install .okapy packages without leaving the game.
- Show download and installation progress and actionable errors.
- View installed mods and their enabled/disabled state.
- Update or remove installed mods.
- Explain missing dependencies, conflicts, and incompatible game versions.
- Offer a recovery route to disable problematic mods on the next launch.

The interface must use Cocos2d-x objects in the game's own scene graph and lifecycle. It must not be implemented as a native Android Activity, overlay, floating window, or WebView.

### 3. Okapy Core

A future native C++ runtime, with a small JNI boundary to the Android application.

Proposed internal services:
- **RuntimeController:** coordinates startup and shutdown states.
- **PackageCatalog:** reads installed package metadata without executing mod code.
- **RepositoryClient:** retrieves catalog metadata and downloads package files.
- **PackageInstaller:** validates and installs downloaded packages into private Okapy storage.
- **DependencyPlanner:** checks required dependencies, version constraints, and conflicts before activation.
- **ModLifecycle:** controls discovery, preparation, activation, failure, and deactivation.
- **NativeModuleBackend:** platform-specific native-library loading behind an interface.
- **CocosAdapter:** creates and manages Okapy's native Cocos2d-x UI and dispatches work to the correct engine thread.
- **GameIntegration:** provides game-specific entry points and UI integration.
- **HookBackend:** a replaceable integration layer; no hook implementation is selected yet.
- **ResourceService:** exposes mod-owned resources through controlled paths.
- **ConfigService:** stores per-mod settings and persistent data.
- **DiagnosticService:** records startup phases, mod errors, and crash context.
- **RecoveryPolicy:** starts without third-party mods when safe mode is requested.

These are design boundaries, not implemented classes yet. Their names and contracts may change as the runtime is prototyped.

### 4. Okapy SDK

A separate developer-facing API and build toolchain. The SDK must define:
- Stable mod metadata and lifecycle contracts.
- Supported game APIs and compatibility declarations.
- Logging, configuration, and resource access.
- Dependency declarations and version constraints.
- A reproducible package-building command.
- Header/API versioning and examples.

The SDK must not expose unstable Core internals as public API.

### 5. .okapy package

The current Draft 0.1 is a ZIP-based package with a root okapy.json manifest and Android ABI-specific native entrypoints. The format is not frozen. Before a stable release, it needs strict JSON rules, dependency constraints, integrity hashes, signing/trust decisions, and explicit compatibility semantics.

A valid manifest is not proof that a native library is safe. Validation, installation, trust decisions, and execution are separate operations.

## In-game installation flow

1. The player opens the Okapy interface from inside Geometry Dash.
2. Okapy requests the repository catalog and displays compatible mods.
3. The player opens a mod's detail page and chooses Install.
4. Okapy downloads the .okapy archive and reports progress.
5. Core validates the archive, game version, ABI, manifest, and dependencies.
6. The package installer extracts into a staging directory and commits the install only after successful extraction.
7. The mod appears in Installed Mods. Activation occurs according to the runtime's lifecycle policy, normally on the next game launch unless safe live activation is explicitly supported.
8. Updates and removals are managed from the same in-game interface.

Downloaded packages must never be executed before validation and an explicit user decision. The in-game catalog must distinguish repository metadata from the actual downloaded package and verify integrity before installation.

## Proposed runtime lifecycle

1. **Bootstrap:** the launcher selects normal or safe mode and passes a validated launch configuration.
2. **Environment check:** Core verifies supported game version, ABI, storage paths, and runtime prerequisites.
3. **Catalog:** installed package metadata is read without loading native code.
4. **Planning:** dependencies, conflicts, enabled state, and compatibility are evaluated as a complete plan.
5. **Preparation:** required files and resources are checked and runtime services are initialized.
6. **Activation:** eligible native modules are loaded in dependency order; each activation is tracked independently.
7. **Cocos integration:** the Cocos adapter becomes ready on the engine thread; the in-game interface is attached to the appropriate game scene/layer.
8. **Running:** the in-game interface, runtime events, and diagnostics are available.
9. **Failure recovery:** a failed mod is isolated where possible; the next launch can use safe mode to skip third-party modules.
10. **Shutdown:** runtime-owned resources and diagnostics are finalized.

No implementation should assume that unloading arbitrary native code is always safe. The first runtime may support disabling a mod for the next launch rather than unloading it live.

## Android integration: unresolved engineering gate

The current launcher only opens the installed game with a normal launch Intent. That cannot, by itself, load a native mod runtime into the game's process or provide direct Cocos2d-x integration.

A future Android integration must be researched and prototyped independently. It may require an Android host Activity, JNI, NDK-built libraries, and a compatible way to initialize the game's native engine and UI. This is a major technical gate: do not implement guessed library names, offsets, patches, or version assumptions. Do not modify the game installation until the integration method, compatibility constraints, and recovery path are understood and tested.

## Security boundaries

- Never extract archive entries using untrusted paths without canonical-path containment checks.
- Enforce compressed and expanded size limits while reading actual bytes.
- Reject duplicate normalized paths and malformed manifests.
- Do not load a package during validation or preview.
- Clearly identify that native mods can execute code with the app's available privileges.
- Require user confirmation before installing or enabling third-party native code.
- Keep safe mode independent from third-party mod state.
- Do not call a mod trusted solely because its package signature or manifest is valid.
- Verify downloaded package integrity against repository metadata or a signed manifest before installation.

## Independence and licensing

Okapy uses Geode as a public technical reference for understanding the problem space and user-facing capabilities. Okapy implementation must be written independently. Do not copy Geode code, assets, SDK declarations, internal names, or package format. When a behavior is useful, document the requirement in general terms and design an Okapy-specific interface and implementation. Keep a record of third-party dependencies and comply with their licenses.

## Current implementation status

- Android launcher foundation, game detection, and ordinary launch Intent: implemented; physical-device verification remains.
- Okapy home screen matching the intended layout: not implemented.
- Direct Cocos2d-x integration, Cocos adapter, and native in-game interface: not implemented.
- .okapy ZIP/manifest validator: initial implementation exists; not yet built or tested.
- .okapy extraction installer: initial implementation exists; not yet built or tested; intended to be called by in-game install flow.
- Repository client, dependency planner, native Core, JNI bridge, SDK, hook backend, and native loading: not implemented.
- Android game-host and Cocos2d-x integration details: not selected or implemented; requires research against supported game binaries/builds.
