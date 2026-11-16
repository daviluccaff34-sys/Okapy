package com.okapy.launcher.mods

import org.json.JSONException
import org.json.JSONObject
import java.io.File
import java.io.IOException

/**
 * File-level management for installed .okapy packages.
 *
 * This class never loads native libraries or enables mods in Geometry Dash.
 * Use an app-private filesDir child as the storage root.
 */
class OkapyModManager(private val storageRoot: File) {
    data class InstalledMod(
        val id: String,
        val name: String,
        val version: String,
        val developer: String,
        val directory: File
    )

    data class OperationResult(
        val succeeded: Boolean,
        val modId: String? = null,
        val message: String
    )

    @Synchronized
    fun install(
        archive: File,
        expectedGameId: String,
        installedGameVersion: String,
        supportedAbis: Set<String>
    ): OperationResult = installInternal(
        archive, expectedGameId, installedGameVersion, supportedAbis, replace = false
    )

    @Synchronized
    fun update(
        archive: File,
        expectedGameId: String,
        installedGameVersion: String,
        supportedAbis: Set<String>
    ): OperationResult = installInternal(
        archive, expectedGameId, installedGameVersion, supportedAbis, replace = true
    )

    @Synchronized
    fun listInstalled(): List<InstalledMod> {
        val root = safeRoot() ?: return emptyList()
        val children = root.listFiles() ?: return emptyList()
        return children.asSequence()
            .filter { it.isDirectory && ModId.isValid(it.name) }
            .mapNotNull { directory ->
                try {
                    val canonical = directory.canonicalFile
                    if (canonical.parentFile != root) return@mapNotNull null
                    val manifestFile = File(canonical, "okapy.json")
                    if (!manifestFile.isFile || manifestFile.length() > MAX_MANIFEST_BYTES) {
                        return@mapNotNull null
                    }
                    val manifest = JSONObject(manifestFile.readText(Charsets.UTF_8))
                    val id = manifest.optString("id", "")
                    if (id != directory.name || !ModId.isValid(id)) return@mapNotNull null
                    InstalledMod(
                        id = id,
                        name = manifest.optString("name", id),
                        version = manifest.optString("version", "unknown"),
                        developer = manifest.optString("developer", ""),
                        directory = canonical
                    )
                } catch (_: IOException) {
                    null
                } catch (_: JSONException) {
                    null
                } catch (_: SecurityException) {
                    null
                }
            }
            .sortedBy { it.id }
            .toList()
    }

    @Synchronized
    fun uninstall(modId: String): OperationResult {
        if (!ModId.isValid(modId)) {
            return OperationResult(false, message = "Invalid mod ID.")
        }
        val root = safeRoot()
            ?: return OperationResult(false, modId, "Mod storage directory is unavailable.")
        val target = File(root, modId)
        return try {
            if (!target.exists()) {
                OperationResult(false, modId, "This mod is not installed.")
            } else if (target.canonicalFile.parentFile != root || !target.isDirectory) {
                OperationResult(false, modId, "Refusing to remove an unsafe mod path.")
            } else if (!target.deleteRecursively()) {
                OperationResult(false, modId, "Could not completely remove the mod directory.")
            } else {
                OperationResult(true, modId, "Mod uninstalled.")
            }
        } catch (_: IOException) {
            OperationResult(false, modId, "Could not access the mod directory.")
        } catch (_: SecurityException) {
            OperationResult(false, modId, "Access to the mod directory was denied.")
        }
    }

    private fun installInternal(
        archive: File,
        expectedGameId: String,
        installedGameVersion: String,
        supportedAbis: Set<String>,
        replace: Boolean
    ): OperationResult {
        val root = safeRoot()
            ?: return OperationResult(false, message = "Could not prepare mod storage.")
        val result = OkapyPackageInstaller.install(
            archive = archive,
            destinationRoot = root,
            expectedGameId = expectedGameId,
            installedGameVersion = installedGameVersion,
            supportedAbis = supportedAbis,
            replaceExisting = replace
        )
        return if (result.isInstalled) {
            OperationResult(
                true,
                result.modId,
                if (replace) "Mod updated." else "Mod installed."
            )
        } else {
            OperationResult(
                false,
                result.modId,
                result.errors.firstOrNull() ?: "Package operation failed."
            )
        }
    }

    private fun safeRoot(): File? {
        return try {
            if (!storageRoot.exists() && !storageRoot.mkdirs()) return null
            val root = storageRoot.canonicalFile
            if (!root.isDirectory) null else root
        } catch (_: IOException) {
            null
        } catch (_: SecurityException) {
            null
        }
    }

    private object ModId {
        private val pattern = Regex("[a-z0-9][a-z0-9._-]{1,127}")
        fun isValid(value: String): Boolean =
            pattern.matches(value) && !value.endsWith(".") &&
                !value.contains("..") && value.split('.').none { it.isEmpty() }
    }

    private companion object {
        const val MAX_MANIFEST_BYTES = 64L * 1024L
    }
}
