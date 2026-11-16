# Okapy Launcher

The first Okapy launcher target is **Android**.

## Android application

The initial project is in `android/` and uses Kotlin with the Android SDK.

### Current v0.1.0 foundation

- Native Android application entry point.
- Basic launcher screen and Okapy version display.
- Checks whether the Geometry Dash Android package is installed.
- Opens the installed game through Android's package launch intent.
- Shows a useful message when the game is not detected.

### Not implemented yet

- Geode or Okapy mod loading.
- Native library injection or binary patching.
- Installation or modification of Geometry Dash files.
- Safe mode for a mod runtime.
- Mod management and `.okapy` package installation.
- Automated game-version compatibility checks.

The current app is an early launcher shell, not yet a mod loader. It does not download Geometry Dash or bypass its official distribution.

## Build

1. Open `android/` in Android Studio.
2. Install Android SDK Platform 35 and JDK 17 if Android Studio requests them.
3. Let Gradle sync finish.
4. Select the `app` run configuration and run it on an Android device or emulator.

The project currently does not include the Gradle wrapper. Android Studio can open the Gradle project directly.

See [Geode research](../docs/GEODE-RESEARCH.md) and [roadmap](../docs/ROADMAP.md).
