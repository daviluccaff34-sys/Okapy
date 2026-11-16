# Okapy roadmap

A feature is complete only after implementation and testing on a documented device and game version.

## v0.1.0 — Android launcher foundation

- [x] Choose Android as the first target platform.
- [x] Create the initial Kotlin Android application project.
- [x] Create the launcher screen.
- [x] Detect the Geometry Dash Android package.
- [x] Add an explicit launch action for an installed game.
- [ ] Build and install a debug APK.
- [ ] Test detection and launching on a real Android device.
- [ ] Improve diagnostics and handle unsupported game versions.

## v0.2.0 — Launcher reliability

- [ ] Add a dedicated diagnostics/log screen.
- [ ] Add persistent launcher settings.
- [ ] Design safe recovery behavior.
- [ ] Add compatibility checks for supported game versions.

## Later: mod runtime

- [ ] Specify the `.okapy` package format and metadata schema.
- [ ] Define mod lifecycle states and error reporting.
- [ ] Design dependency and incompatibility handling.
- [ ] Research Android-specific native loading and integration requirements.
- [ ] Build and test a minimal example mod before claiming mod support.

## Later: developer tools and distribution

- [ ] Define the Okapy SDK and project template.
- [ ] Create package validation/build tooling.
- [ ] Build the download website at https://okapy.pages.dev.
- [ ] Publish versioned releases with checksums.

## Status meanings

- Planned: documented, not implemented.
- In progress: implementation has started.
- Implemented: code exists, testing may remain.
- Tested: verified on the documented device and game version.
