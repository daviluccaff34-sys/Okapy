# Okapy roadmap

A feature is not complete until it is implemented and tested on a documented platform.

## v0.1.0 — Launcher foundation

- [ ] Choose and document the first supported platform.
- [ ] Create the launcher application shell.
- [ ] Detect and validate a supported Geometry Dash installation.
- [ ] Show game installation and version status.
- [ ] Provide an explicit launch action.
- [ ] Add a recovery/safe mode where the platform permits it.
- [ ] Add diagnostic logs and useful error messages.
- [ ] Define backup and recovery behavior before changing user game data.
- [ ] Document tested platforms and supported game versions.

## Later: mod runtime

- [ ] Specify the `.okapy` package format and metadata schema.
- [ ] Define mod lifecycle states and error reporting.
- [ ] Design dependency and incompatibility handling.
- [ ] Research platform-specific binary loading and hook mechanisms.
- [ ] Build and test a minimal example mod.

## Later: developer tools and distribution

- [ ] Define the Okapy SDK and project template.
- [ ] Create package validation/build tooling.
- [ ] Build the download website at https://okapy.pages.dev.
- [ ] Publish versioned releases with checksums.

## Status meanings

- Planned: documented, not implemented.
- In progress: implementation has started.
- Implemented: code exists, testing may remain.
- Tested: verified on the documented platform/device/game version.
