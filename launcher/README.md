# Okapy Launcher

This directory is reserved for the Okapy launcher application.

## v0.1.0 goal

Build a reliable launcher foundation first. The first release must clearly state its supported platform and Geometry Dash versions, validate the game installation before launch, and provide understandable diagnostics.

## Responsibilities

- Detect a supported Geometry Dash installation.
- Check compatibility and required files.
- Prepare Okapy integration only after the platform-specific design is established.
- Start the game using a platform-appropriate mechanism.
- Show launch errors and collect diagnostic logs.
- Provide a recovery path if launch fails.

## Current status

This is a planning document only. No executable launcher, game integration, injection mechanism, or mod runtime has been implemented here yet.

## Platform decision

The initial target platform is not decided yet. Platform-specific source code and build configuration should be added after this decision because Android and desktop systems have different installation, permission, and launch models.

See [Geode research](../docs/GEODE-RESEARCH.md) and [roadmap](../docs/ROADMAP.md).
