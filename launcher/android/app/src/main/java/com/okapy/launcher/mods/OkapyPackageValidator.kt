package com.okapy.launcher.mods

import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.io.File
import java.io.IOException
import java.util.zip.ZipFile

/**
 * Performs structural and compatibility checks on a .okapy archive.
 *
 * This validator does not load or execute native libraries. A successful result
 * means only that the package passed these checks.
 */
object OkapyPackageValidator {
    private const val MANIFEST_NAME = "okapy.json"
    private const val MAX_MANIFEST_BYTES = 64L * 1024L
    private const val MAX_ARCHIVE_ENTRIES = 2_048
    private const val MAX_UNCOMPRESSED_BYTES = 256L * 1024L * 1024L
    private val idPattern = Regex("[a-z0-9][a-z0-9._-]{1,127}")

    data class ValidationResult(
        val isValid: Boolean,
        val modId: String? = null,
        val modName: String? = null,
        val modVersion: String? = null,
        val entrypointPath: String? = null,
        val errors: List<String> = emptyList()
    )

    fun validate(
        archive: File,
        expectedGameId: String,
        installedGameVersion: String,
        supportedAbis: Set<String>
    ): ValidationResult {
        if (!archive.isFile || !archive.canRead()) {
            return failure("Package file does not exist or cannot be read.")
        }
        if (!archive.name.endsWith(".okapy", ignoreCase = true)) {
            return failure("Package extension must be .okapy.")
        }

        try {
            ZipFile(archive).use { zip ->
                val entries = zip.entries()
                val entryNames = HashSet<String>()
                var entryCount = 0
                var expandedBytes = 0L

                while (entries.hasMoreElements()) {
                    val entry = entries.nextElement()
                    entryCount++
                    if (entryCount > MAX_ARCHIVE_ENTRIES) {
                        return failure("Package contains too many files.")
                    }
                    if (!isSafeArchivePath(entry.name, entry.isDirectory)) {
                        return failure("Package contains an unsafe archive path.")
                    }
                    if (!entryNames.add(entry.name)) {
                        return failure("Package contains duplicate archive paths.")
                    }
                    if (entry.size > 0) {
                        expandedBytes += entry.size
                        if (expandedBytes > MAX_UNCOMPRESSED_BYTES) {
                            return failure("Package expands beyond the allowed size.")
                        }
                    }
                }

                val manifestEntry = zip.getEntry(MANIFEST_NAME)
                    ?: return failure("Root manifest okapy.json is missing.")
                if (manifestEntry.isDirectory || manifestEntry.size < 0 ||
                    manifestEntry.size > MAX_MANIFEST_BYTES
                ) {
                    return failure("Manifest is not a valid size.")
                }

                val manifestText = zip.getInputStream(manifestEntry).bufferedReader(Charsets.UTF_8).use {
                    it.readText()
                }
                val manifest = try {
                    JSONObject(manifestText)
                } catch (_: JSONException) {
                    return failure("Manifest contains invalid JSON.")
                }

                val formatVersion = manifest.optInt("formatVersion", -1)
                if (formatVersion != 1) {
                    return failure("Unsupported package format version.")
                }

                val modId = manifest.optString("id", "")
                val modName = manifest.optString("name", "")
                val modVersion = manifest.optString("version", "")
                val developer = manifest.optString("developer", "")
                if (!idPattern.matches(modId)) return failure("Mod ID is missing or invalid.")
                if (modName.isBlank()) return failure("Mod name is missing.")
                if (modVersion.isBlank()) return failure("Mod version is missing.")
                if (developer.isBlank()) return failure("Developer name is missing.")

                val game = manifest.optJSONObject("game")
                    ?: return failure("Game compatibility information is missing.")
                if (game.optString("id") != expectedGameId) {
                    return failure("This mod targets a different game package.")
                }
                val versions = game.optJSONArray("versions")
                    ?: return failure("Supported game versions are missing.")
                if (!containsString(versions, installedGameVersion)) {
                    return failure("This mod does not support the installed game version.")
                }

                val android = manifest.optJSONObject("entrypoints")
                    ?.optJSONObject("android")
                    ?: return failure("Android entrypoints are missing.")
                val selectedEntrypoint = supportedAbis.firstNotNullOfOrNull { abi ->
                    val path = android.optString(abi, "").takeIf { it.isNotBlank() }
                    if (path != null && entryNames.contains(path) &&
                        zip.getEntry(path)?.isDirectory == false
                    ) abi to path else null
                } ?: return failure("No valid native entrypoint matches this device.")

                val dependencies = manifest.optJSONArray("dependencies")
                if (dependencies != null && !validIdArray(dependencies)) {
                    return failure("Dependency list contains an invalid mod ID.")
                }
                val conflicts = manifest.optJSONArray("conflicts")
                if (conflicts != null && !validIdArray(conflicts)) {
                    return failure("Conflict list contains an invalid mod ID.")
                }

                return ValidationResult(
                    isValid = true,
                    modId = modId,
                    modName = modName,
                    modVersion = modVersion,
                    entrypointPath = selectedEntrypoint.second
                )
            }
        } catch (_: IOException) {
            return failure("Package is not a readable ZIP archive.")
        } catch (_: SecurityException) {
            return failure("Package access was denied.")
        }
    }

    private fun isSafeArchivePath(path: String, isDirectory: Boolean): Boolean {
        if (path.isBlank() || path.startsWith("/") || path.startsWith("\\") ||
            path.contains('\\') || path.contains(':')
        ) return false
        val normalized = if (isDirectory) path.removeSuffix("/") else path
        if (normalized.isBlank()) return false
        val segments = normalized.split("/")
        return segments.none { it == ".." || it == "." || it.isEmpty() }
    }

    private fun containsString(array: JSONArray, expected: String): Boolean {
        for (index in 0 until array.length()) {
            if (array.optString(index) == expected) return true
        }
        return false
    }

    private fun validIdArray(array: JSONArray): Boolean {
        for (index in 0 until array.length()) {
            val value = array.optString(index, "")
            if (!idPattern.matches(value)) return false
        }
        return true
    }

    private fun failure(message: String) = ValidationResult(
        isValid = false,
        errors = listOf(message)
    )
}
