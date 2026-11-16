package com.okapy.launcher.mods

import java.io.File
import java.io.IOException
import java.util.UUID
import java.util.zip.ZipFile

/**
 * Extracts a previously validated .okapy package into Okapy's private mod directory.
 *
 * This class installs files only. It does not resolve dependencies or execute native code.
 */
object OkapyPackageInstaller {
    private class InstallFailure(message: String) : Exception(message)

    private const val MAX_ARCHIVE_ENTRIES = 2_048
    private const val MAX_UNCOMPRESSED_BYTES = 256L * 1024L * 1024L

    data class InstallResult(
        val isInstalled: Boolean,
        val modId: String? = null,
        val modName: String? = null,
        val modVersion: String? = null,
        val installedDirectory: File? = null,
        val errors: List<String> = emptyList()
    )

    /**
     * Validates the package again immediately before extraction.
     * The destination should be a private app-internal directory such as filesDir/mods.
     */
    @Synchronized
    fun install(
        archive: File,
        destinationRoot: File,
        expectedGameId: String,
        installedGameVersion: String,
        supportedAbis: Set<String>
    ): InstallResult {
        val validation = OkapyPackageValidator.validate(
            archive = archive,
            expectedGameId = expectedGameId,
            installedGameVersion = installedGameVersion,
            supportedAbis = supportedAbis
        )
        if (!validation.isValid) {
            return failure(validation.errors.firstOrNull() ?: "Package validation failed.")
        }

        val modId = validation.modId ?: return failure("Validated package has no mod ID.")
        val root = try {
            if (!destinationRoot.exists() && !destinationRoot.mkdirs()) {
                return failure("Could not create the mod storage directory.")
            }
            destinationRoot.canonicalFile
        } catch (_: IOException) {
            return failure("Could not access the mod storage directory.")
        } catch (_: SecurityException) {
            return failure("Access to the mod storage directory was denied.")
        }

        if (!root.isDirectory) return failure("Mod storage path is not a directory.")

        val target = File(root, modId)
        if (target.exists()) {
            return failure("A mod with this ID is already installed.")
        }

        val staging = File(root, ".install-${UUID.randomUUID()}")
        if (!staging.mkdir()) {
            return failure("Could not create a temporary installation directory.")
        }

        try {
            ZipFile(archive).use { zip ->
                val entries = zip.entries()
                val normalizedNames = HashSet<String>()
                val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
                var entryCount = 0
                var expandedBytes = 0L

                while (entries.hasMoreElements()) {
                    val entry = entries.nextElement()
                    entryCount++
                    if (entryCount > MAX_ARCHIVE_ENTRIES) {
                        throw InstallFailure("Package contains too many files.")
                    }

                    val normalizedPath = normalizeSafePath(entry.name, entry.isDirectory)
                        ?: throw InstallFailure("Package contains an unsafe archive path.")
                    if (!normalizedNames.add(normalizedPath)) {
                        throw InstallFailure("Package contains duplicate normalized paths.")
                    }

                    val output = File(staging, normalizedPath)
                    val canonicalOutput = output.canonicalFile
                    val stagingPrefix = staging.canonicalPath + File.separator
                    if (!canonicalOutput.path.startsWith(stagingPrefix)) {
                        throw InstallFailure("Package entry escapes the installation directory.")
                    }

                    if (entry.isDirectory) {
                        if (output.exists() && !output.isDirectory) {
                            throw InstallFailure("Package contains a file/directory path collision.")
                        }
                        if (!output.exists() && !output.mkdirs()) {
                            throw InstallFailure("Could not create a package directory.")
                        }
                        continue
                    }

                    val parent = output.parentFile
                        ?: throw InstallFailure("Package entry has no parent directory.")
                    if (parent.exists() && !parent.isDirectory) {
                        throw InstallFailure("Package contains a file/directory path collision.")
                    }
                    if (!parent.exists() && !parent.mkdirs()) {
                        throw InstallFailure("Could not create a package directory.")
                    }
                    if (output.exists()) {
                        throw InstallFailure("Package contains duplicate output paths.")
                    }

                    zip.getInputStream(entry).use { input ->
                        output.outputStream().use { stream ->
                            while (true) {
                                val count = input.read(buffer)
                                if (count == -1) break
                                expandedBytes += count
                                if (expandedBytes > MAX_UNCOMPRESSED_BYTES) {
                                    throw InstallFailure("Package expands beyond the allowed size.")
                                }
                                stream.write(buffer, 0, count)
                            }
                        }
                    }
                }
            }

            // Staging and destination are siblings, so this rename stays on the same filesystem.
            // The synchronized installer also prevents two in-process installs racing for one ID.
            if (target.exists() || !staging.renameTo(target)) {
                return abort(staging, "Could not finalize the package installation.")
            }

            return InstallResult(
                isInstalled = true,
                modId = validation.modId,
                modName = validation.modName,
                modVersion = validation.modVersion,
                installedDirectory = target
            )
        } catch (failure: InstallFailure) {
            return abort(staging, failure.message ?: "Package installation failed.")
        } catch (_: IOException) {
            return abort(staging, "Package could not be extracted as a readable ZIP archive.")
        } catch (_: SecurityException) {
            return abort(staging, "Package extraction access was denied.")
        }
    }

    private fun normalizeSafePath(path: String, isDirectory: Boolean): String? {
        if (path.isBlank() || path.startsWith("/") || path.startsWith("\\") ||
            path.contains('\\') || path.contains(':')
        ) return null

        val normalized = if (isDirectory) path.removeSuffix("/") else path
        if (normalized.isBlank()) return null

        val segments = normalized.split("/")
        if (segments.any { it.isEmpty() || it == "." || it == ".." }) return null
        return segments.joinToString("/")
    }

    private fun abort(staging: File, message: String): InstallResult {
        staging.deleteRecursively()
        return failure(message)
    }

    private fun failure(message: String) = InstallResult(
        isInstalled = false,
        errors = listOf(message)
    )
}
