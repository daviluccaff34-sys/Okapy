# Okapy roadmap

A feature is complete only after implementation and testing on a documented device and game version. APK releases are not published just to mark development milestones.

## Version plan

- **0.00.1** — initial development version.
- **0.01.1 Beta** — first Beta release, after the planned launcher and mod-loading features are implemented and tested.
- Later versions will be defined as development progresses.

## v0.00.1 — initial development

- [x] Choose Android as the first target platform.
- [x] Create the initial Kotlin Android application project.
- [x] Create the launcher screen.
- [x] Detect the Geometry Dash Android package.
- [x] Add an explicit launch action for an installed game.
- [ ] Continue implementing the complete launcher and mod-loading architecture.

## v0.01.1 Beta — first Beta

- [ ] Complete the launcher experience.
- [x] Draft the initial `.okapy` package format and metadata schema (not yet frozen).
- [x] Add the first Android ZIP/manifest validator (implementation exists; testing remains).
- [ ] Test and harden the package validator.
- [ ] Implement mod discovery, installation, loading, and management.
- [ ] Implement dependency and incompatibility handling.
- [ ] Add safe recovery behavior, diagnostics, and logs.
- [ ] Verify compatibility with documented Geometry Dash versions.
- [ ] Test the complete application on real Android devices.
- [ ] Fix release-blocking issues before publishing the Beta APK.

## Developer tools and distribution

- [ ] Define the Okapy SDK and project template.
- [ ] Create package validation/build tooling.
- [ ] Build the download website at https://okapy.pages.dev.
- [ ] Publish versioned releases with checksums.

## Status meanings

- Planned: documented, not implemented.
- In progress: implementation has started.
- Implemented: code exists, testing may remain.
- Tested: verified on the documented device and game version.
