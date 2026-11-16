# Okapy

Independent Android launcher and in-game mod-loading platform for Geometry Dash.

## About

Okapy is being developed as its own platform, with a simple Android launcher, an in-game mod management interface, a native runtime (Okapy Core), a developer SDK, and the .okapy mod package format. Geode is used as a public architectural reference for studying the problem space; Okapy's implementation, code, APIs, package rules, and identity are independent.

## Product experience

- The Android launcher will have an original Okapy identity, a centered logo, a prominent Launch button, and a Settings control.
- Mods will be discovered, installed, updated, enabled, disabled, and removed from inside Geometry Dash through the Okapy in-game interface.
- The launcher is for starting the game and recovery/settings, not the main mod store.

## Version

The initial development version is **0.00.1**. The first Beta is planned as **0.01.1**. An APK will be published only after its intended features are implemented and tested.

## Current state

- Android launcher foundation: implemented.
- Geometry Dash package detection: implemented.
- Ordinary launch of the installed game: implemented.
- Intended home screen: not implemented.
- In-game mod interface and repository browser: not implemented.
- .okapy ZIP/manifest validator: initial implementation exists, not yet built or tested.
- .okapy extraction installer: initial implementation exists, not yet built or tested; intended for the in-game installation flow.
- Repository client, mod catalog, dependency planner, and enable/disable management: not implemented.
- Native runtime and native loading: not implemented.
- SDK and developer tooling: not implemented.
- Android game-host and in-game UI integration: still under technical investigation.

See [the architecture design](docs/OKAPY-ARCHITECTURE.md), [package format draft](docs/OKAPY-PACKAGE-FORMAT.md), and [Geode research notes](docs/GEODE-RESEARCH.md).

## Android project

The Android Studio project is in [launcher/android](launcher/android). Build instructions are in [launcher/README.md](launcher/README.md).

## Downloads

The separate downloads website is planned at https://okapy.pages.dev.

## License

This repository is licensed under the MIT License. See [LICENSE](LICENSE).
