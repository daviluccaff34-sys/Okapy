package com.okapy.launcher

/**
 * Small JNI boundary for the native Okapy Core.
 *
 * This initializes the Core library only. It does not inject code into
 * Geometry Dash or load installed mods into the game process.
 */
object OkapyNative {
    val isAvailable: Boolean

    init {
        isAvailable = try {
            System.loadLibrary("okapy_jni")
            true
        } catch (_: UnsatisfiedLinkError) {
            false
        } catch (_: SecurityException) {
            false
        }
    }

    external fun nativeVersion(): String
    external fun nativeInitialize(): Boolean
    external fun nativeShutdown()
    external fun nativeStatus(): String

    fun versionOrFallback(): String =
        if (isAvailable) nativeVersion() else "Native library unavailable"

    fun statusOrFallback(): String =
        if (isAvailable) nativeStatus() else "JNI bridge unavailable"
}
