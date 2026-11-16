# Okapy

Independent launcher and future mod-loading platform for Geometry Dash.

## About

Okapy is being developed as its own platform, with an Android launcher, a native runtime (Okapy Core), a developer SDK, and the .okapy mod package format. Geode is used as a public architectural reference for studying the problem space; Okapy's implementation, code, APIs, package rules, and identity are independent.

## Version

The initial development version is **0.00.1**. The first Beta is planned as **0.01.1**. An APK will be published only after its intended features are implemented and tested.

## Current state

- Android launcher foundation: implemented.
- Geometry Dash package detection: implemented.
- Ordinary launch of the installed game: implemented.
- .okapy ZIP/manifest validator: initial implementation exists, not yet built or tested.
- Mod installation and management: not implemented.
- Native runtime, dependency planner, and native loading: not implemented.
- SDK and developer tooling: not implemented.
- Android game-host integration: still under technical investigation.

See [the architecture design](docs/OKAPY-ARCHITECTURE.md), [package format draft](docs/OKAPY-PACKAGE-FORMAT.md), and [Geode research notes](docs/GEODE-RESEARCH.md).

## Android project

The Android Studio project is in [launcher/android](launcher/android). Build instructions are in [launcher/README.md](launcher/README.md).

## Downloads

The separate downloads website is planned at https://okapy.pages.dev.

## License

This repository is licensed under the MIT License. See [LICENSE](LICENSE).
