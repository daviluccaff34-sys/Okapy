# Okapy package format (.okapy)

**Status:** Draft 0.1 — specification draft; initial Android validation and extraction implementations exist but have not yet been built or tested.

This document defines the first proposed structure for Okapy mod packages. The format must remain explicit, inspectable, and versioned so the launcher can reject packages it cannot safely understand.

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

The archive filename is not the mod identity. The stable identity comes from id in the manifest.

## 2. Manifest

The manifest is UTF-8 JSON. Initial proposed fields:

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
  "dependencies": [],
  "conflicts": []
}
```

### Field rules

- formatVersion: required integer; identifies the package schema, not the mod release.
- id: required, globally unique reverse-domain-style identifier; lowercase letters, digits, dots, hyphens, and underscores only.
- name, version, and developer: required non-empty strings.
- description: optional plain text.
- game.id: required game package identifier.
- game.versions: required list of supported game versions; an empty list must not mean "all versions".
- entrypoints.android: map of Android ABI names to library paths inside the archive.
- dependencies: list of required mod IDs and version constraints.
- conflicts: list of mod IDs that cannot be enabled alongside this mod.

The exact version-constraint syntax and dependency-resolution rules still need a separate specification.

## 3. Validation and installation requirements

Before a package can be installed, Okapy must:

1. Check that the file is a valid ZIP and that it contains one root okapy.json.
2. Parse JSON and reject unknown or unsupported formatVersion values.
3. Validate required fields, identifier syntax, and version strings.
4. Ensure every entrypoint path refers to a file inside the archive.
5. Reject absolute paths, parent-directory traversal (..), duplicate normalized paths, and unsafe archive entries.
6. Enforce package entry-count and expanded-size limits while reading actual bytes.
7. Check game ID/version and device ABI compatibility.
8. Detect duplicate mod IDs and unresolved dependencies/conflicts before activation.
9. Verify file hashes when package integrity metadata is added to a later revision.

A package must never be loaded merely because its extension is .okapy.

## 4. Security and trust

Native mod libraries are executable code. Installing or enabling a mod must require clear user consent. The installer currently only extracts files after validation; the launcher UI must provide the user-facing confirmation before calling it. Okapy must not describe a mod as safe merely because its manifest is valid.

The design still needs a trust/signing model, permission model, and a safe way to disable a mod that prevents startup.

## 5. Compatibility and implementation status

This is a draft for discussion and implementation planning. It is not yet a stable public specification. Field names, ABI support, signing, dependency constraints, and the native loading contract may change before the first Beta.

The initial Android archive/manifest validator is in `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyPackageValidator.kt`. The initial extraction installer is in `launcher/android/app/src/main/java/com/okapy/launcher/mods/OkapyPackageInstaller.kt`. The validator checks ZIP structure, manifest fields, game compatibility, ABI entrypoint, dependency/conflict ID syntax, and unsafe paths. The installer revalidates, extracts into a temporary private directory, checks normalized paths and size limits, and then renames the staging directory into place. Neither implementation has been built or tested. The installer is not yet connected to the launcher UI. Dependency resolution, package signing, and native loading are not implemented.
