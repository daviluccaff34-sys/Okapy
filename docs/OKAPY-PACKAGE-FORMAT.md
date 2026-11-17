# Okapy package format (.okapy)

**Status:** Draft 0.1 — Android archive installation, native manifest parsing, installed entrypoint validation, and explicit Core activation orchestration are implemented in source. No build or test has been run for this revision.

This document defines the first proposed structure for Okapy mod packages. The format must remain explicit, inspectable, and versioned so the in-game Okapy interface can reject packages it cannot safely understand.

## 1. Container

A .okapy file is a ZIP archive. The archive must contain exactly one manifest at its root:

```text
example-mod.okapy
├── okapy.json
├── libs/
│   ├── arm64-v8a/
│   │   └── libexample.so
│   └── armeabi-v7a/
│       └── libexample.so
└── assets/
    └── ...
```

The archive filename is not the mod identity. The stable identity comes from `id` in the manifest.

## 2. Manifest

The manifest is UTF-8 JSON. Initial schema:

```json
{
  "formatVersion": 1,
  "id": "author.example-mod",
  "name": "Example Mod",
  "version": "1.0.0",
  "developer": "Example Author",
  "description": "A short description of the mod.",
  "game": {
    "id": "com.robtopx.geometryjump",
    "versions": ["2.2"]
  },
  "entrypoints": {
    "android": {
      "arm64-v8a": "libs/arm64-v8a/libexample.so",
      "armeabi-v7a": "libs/armeabi-v7a/libexample.so"
    }
  },
  "dependencies": ["author.library"],
  "conflicts": ["author.incompatible"]
}
```

### Field rules

- `formatVersion`: required JSON integer; currently only `1` is supported.
- `id`: required, globally unique mod identifier; lowercase letters, digits, dots, hyphens, and underscores only.
- `name`, `version`, and `developer`: required non-empty strings.
- `description`: optional string.
- `game.id`: required game package identifier.
- `game.versions`: required non-empty list of supported game-version strings.
- `entrypoints.android`: required map of supported Android ABI names to safe, package-relative library paths. The parser rejects absolute paths, backslashes, empty path segments, `.`, `..`, and drive-letter/colon paths.
- `dependencies`: optional array of mod ID strings. In format 1, dependencies are required dependencies with no minimum-version constraint.
- `conflicts`: optional array of mod ID strings.

The dependency representation intentionally matches the Android validator's current format-1 behavior. Optional dependencies and minimum-version constraints are not part of format 1 yet. Format 1 also has no declared-permissions field; permission grants therefore remain explicit host-side decisions and are denied by default.

## 3. Validation and installation requirements

Before installation, Okapy must:

1. Check that the downloaded file is a valid ZIP and that it contains one root `okapy.json`.
2. Parse JSON and reject unknown or unsupported `formatVersion` values.
3. Validate required fields, identifier syntax, and field types.
4. Ensure every entrypoint path refers to a file inside the archive.
5. Reject absolute paths, parent-directory traversal (`..`), duplicate normalized paths, and unsafe archive entries.
6. Enforce package entry-count and expanded-size limits while reading actual bytes.
7. Check game ID/version and device ABI compatibility.
8. Detect duplicate mod IDs and unresolved dependencies/conflicts before activation.
9. Verify file hashes against repository metadata or signed package metadata before installation.

The in-game interface must show package details and obtain the player's confirmation before installing or enabling native code. A package must never be loaded merely because its extension is `.okapy`.

## 4. Security and trust

Native mod libraries are executable code. Installing or enabling a mod must require clear user consent. Okapy must not describe a mod as safe merely because its manifest is valid. The design still needs a trust/signing model and a safe way to disable a mod that prevents startup.

The Core catalog refuses package update and removal while a mod's registry state is `Enabled`. This is a metadata-level guard only: changing the registry state does not stop an already-running native module. The runtime must stop a module successfully before marking it disabled and permitting package replacement or deletion. The current implementation does not yet expose safe live unload; disabling a running native module must therefore be treated as a next-launch operation until that mechanism exists.

## 5. Native parser, discovery, and activation

The native parser is declared in `core/include/okapy/package_manifest.hpp` and implemented in `core/src/package_manifest.cpp`. It uses a bounded, self-contained JSON parser (64 KiB input limit and 32-level nesting limit), rejects duplicate object keys, validates required manifest fields, decodes JSON Unicode escapes, and maps metadata into `ModInfo` and `ModManifest`. It also exposes the game compatibility strings and Android ABI entrypoint paths.

`InstalledPackageStore::discoverInstalled()` scans direct child package directories under its configured root, reads only a regular root `okapy.json` (without following a manifest symlink), parses it, requires the manifest ID to match the directory name, and registers metadata in the in-memory catalog.

`InstalledPackageStore::entrypointPath(modId, abi)` reparses the installed manifest and returns a canonical entrypoint path only if the declared file exists as a regular file inside the package; symlink components are rejected. `Core::loadInstalledMod()` passes that path to `NativeModLoader` and checks that the library-reported mod ID matches the manifest ID. Loading and querying a native library executes code from that library.

`Core::enableInstalledMod(modId, abi, userConsent)` is the explicit activation entrypoint. It refuses activation unless consent is true, resolves dependencies against a temporary registry (so the selected mod is not marked enabled before startup succeeds), loads the resolved dependency chain in dependency-first order, starts the lifecycle plan, and only then marks the selected mod enabled. Dependencies must already be enabled in the catalog. Lifecycle startup is incremental: previously running mods remain active, and a failed later activation rolls back only the modules started by that call. A failed native load can leave an already-query-registered dependency library resident but not running; it is reused on a later attempt.

The current Core caller supplies the Android ABI explicitly. Automatic process-ABI detection, current Geometry Dash version verification, package signature/hash verification, persistent catalog state, Kotlin/JNI catalog integration, repository downloads, and the in-game Cocos2d-x interface are not implemented.

The manifest does not yet declare requested permissions. Existing Core host callbacks enforce the PermissionManager's default-deny grants at runtime; no permission is granted by activation. A complete permission review screen requires a future package-format extension that declares requested permissions.

## 6. Android implementation status

The Android archive/manifest validator is in `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyPackageValidator.kt`. The extraction implementation is in `OkapyPackageInstaller.kt`; it revalidates, extracts into a staging directory, enforces entry/expanded-size limits and safe paths, then renames the staging directory into place. The management API is in `OkapyModManager.kt`: it exposes install, update, listInstalled, and uninstall operations against an app-private storage root.

Repository downloads, package signing, safe live deactivation, and the native in-game Cocos2d-x management interface are not implemented.
