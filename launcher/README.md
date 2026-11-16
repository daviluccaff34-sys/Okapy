# Okapy Launcher

The first Okapy launcher target is **Android**.

## Android application

The initial project is in `android/` and uses Kotlin with the Android SDK and NDK.

### Current Android app foundation

- Native Android application entry point.
- Basic launcher screen and Okapy version display.
- Checks whether the Geometry Dash Android package is installed.
- Opens the installed game through Android's package launch intent.
- Shows a useful message when the game is not detected.
- Imports, validates, installs, lists, updates, and uninstalls `.okapy` packages in app-private storage.
- Builds the existing C++17 Core for Android through CMake/NDK.
- Provides an initial JNI bridge to initialize, inspect, and shut down the native Core.
- Displays native Core availability, status, and version in the launcher.

The JNI bridge currently exposes the Core lifecycle and basic diagnostics. It does not inject Okapy into Geometry Dash, hook the game's process, or load installed mods into the game.

### Not implemented yet

- Okapy mod loading inside Geometry Dash.
- Native library injection or binary patching.
- Installation or modification of Geometry Dash files.
- Safe mode for a mod runtime.

Package import checks the installed game's package ID, version name, and supported Android ABIs. This is package management only: importing a package does not execute its native entrypoints. The app does not download Geometry Dash or bypass its official distribution.

## Build

1. Open `android/` in Android Studio.
2. Install Android SDK Platform 35, Android NDK, CMake 3.22.1, and JDK 17 if Android Studio requests them.
3. Let Gradle sync finish.
4. Select the `app` run configuration and run it on an Android device or emulator.

The project currently does not include the Gradle wrapper. Android Studio can open the Gradle project directly.

See [Geode research](../docs/GEODE-RESEARCH.md) and [roadmap](../docs/ROADMAP.md).
