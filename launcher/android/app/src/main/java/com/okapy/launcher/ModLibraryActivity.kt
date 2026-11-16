package com.okapy.launcher

import android.app.Activity
import android.content.Intent
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import com.okapy.launcher.mods.OkapyModManager
import java.io.File
import java.io.FileOutputStream

class ModLibraryActivity : Activity() {
    private lateinit var listContainer: LinearLayout
    private lateinit var manager: OkapyModManager

    private val gamePackage = "com.robtopx.geometryjump"
    private val requestOpenPackage = 4101

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        manager = OkapyModManager(File(filesDir, "mods"))
        buildScreen()
        refreshMods()
    }

    private fun buildScreen() {
        val page = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(24), dp(20), dp(24))
            setBackgroundColor(Color.rgb(16, 24, 32))
        }
        val scroll = ScrollView(this).apply {
            isFillViewport = true
            addView(page)
        }

        page.addView(TextView(this).apply {
            text = "MOD LIBRARY"
            textSize = 26f
            setTextColor(Color.WHITE)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
        })
        page.addView(TextView(this).apply {
            text = "Install and manage .okapy packages"
            textSize = 14f
            setTextColor(Color.rgb(151, 169, 181))
        }, params(0, 6))

        page.addView(Button(this).apply {
            text = "Import .okapy package"
            isAllCaps = false
            setOnClickListener { openPackagePicker() }
        }, params(0, 20))

        page.addView(Button(this).apply {
            text = "Refresh list"
            isAllCaps = false
            setOnClickListener { refreshMods() }
        }, params(0, 8))

        page.addView(TextView(this).apply {
            text = "INSTALLED MODS"
            textSize = 11f
            letterSpacing = 0.12f
            setTextColor(Color.rgb(112, 225, 193))
        }, params(0, 24))

        listContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
        }
        page.addView(listContainer, params(0, 10))

        page.addView(TextView(this).apply {
            text = "Packages are stored privately by Okapy. Importing a package does not execute its native code or load it into Geometry Dash yet."
            textSize = 12f
            setTextColor(Color.rgb(151, 169, 181))
        }, params(0, 22))

        setContentView(scroll)
    }

    private fun openPackagePicker() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "application/octet-stream"
            putExtra(Intent.EXTRA_MIME_TYPES, arrayOf(
                "application/zip",
                "application/octet-stream"
            ))
        }
        startActivityForResult(intent, requestOpenPackage)
    }

    @Deprecated("Uses the platform document picker for compatibility with minSdk 23.")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != requestOpenPackage || resultCode != RESULT_OK) return
        val uri = data?.data ?: return
        val temporary = File(cacheDir, "import-${System.currentTimeMillis()}.okapy")
        try {
            contentResolver.openInputStream(uri)?.use { input ->
                FileOutputStream(temporary).use { output -> input.copyTo(output) }
            } ?: throw IllegalStateException("Could not read selected file.")

            val gameInfo = packageManager.getPackageInfo(gamePackage, 0)
            val gameVersion = gameInfo.versionName
                ?: throw IllegalStateException("Could not determine Geometry Dash version.")
            val result = manager.install(
                archive = temporary,
                expectedGameId = gamePackage,
                installedGameVersion = gameVersion,
                supportedAbis = Build.SUPPORTED_ABIS.toSet()
            )
            Toast.makeText(this, result.message, Toast.LENGTH_LONG).show()
            refreshMods()
        } catch (error: Exception) {
            Toast.makeText(
                this,
                error.message ?: "Package import failed.",
                Toast.LENGTH_LONG
            ).show()
        } finally {
            temporary.delete()
        }
    }

    private fun refreshMods() {
        if (!::listContainer.isInitialized) return
        listContainer.removeAllViews()
        val mods = manager.listInstalled()
        if (mods.isEmpty()) {
            listContainer.addView(TextView(this).apply {
                text = "No mods installed yet."
                textSize = 14f
                setTextColor(Color.rgb(225, 233, 238))
                setPadding(0, dp(12), 0, dp(12))
            })
            return
        }

        mods.forEach { mod ->
            val card = LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(16), dp(14), dp(16), dp(14))
                setBackgroundColor(Color.rgb(27, 40, 51))
            }
            card.addView(TextView(this).apply {
                text = mod.name
                textSize = 17f
                setTextColor(Color.WHITE)
                typeface = android.graphics.Typeface.DEFAULT_BOLD
            })
            card.addView(TextView(this).apply {
                text = "${mod.id}  •  v${mod.version}\nby ${mod.developer}"
                textSize = 12f
                setTextColor(Color.rgb(151, 169, 181))
            }, params(0, 5))
            card.addView(Button(this).apply {
                text = "Uninstall"
                isAllCaps = false
                setOnClickListener {
                    val result = manager.uninstall(mod.id)
                    Toast.makeText(this@ModLibraryActivity, result.message, Toast.LENGTH_SHORT).show()
                    refreshMods()
                }
            }, params(0, 8))
            listContainer.addView(card, params(0, 0))
        }
    }

    private fun params(left: Int, top: Int) = LinearLayout.LayoutParams(
        LinearLayout.LayoutParams.MATCH_PARENT,
        LinearLayout.LayoutParams.WRAP_CONTENT
    ).apply {
        if (left != 0 || top != 0) setMargins(dp(left), dp(top), 0, 0)
    }

    private fun dp(value: Int): Int =
        (value * resources.displayMetrics.density).toInt()
}
