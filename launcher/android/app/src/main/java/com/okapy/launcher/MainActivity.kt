package com.okapy.launcher

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast

class MainActivity : Activity() {
    private val gamePackage = "com.robtopx.geometryjump"
    private lateinit var gameStatus: TextView
    private lateinit var diagnosticText: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = Color.rgb(16, 24, 32)
        window.navigationBarColor = Color.rgb(16, 24, 32)
        buildScreen()
        refreshGameStatus()
    }

    override fun onResume() {
        super.onResume()
        if (::gameStatus.isInitialized) refreshGameStatus()
    }

    private fun buildScreen() {
        val page = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(22), dp(28), dp(22), dp(24))
            setBackgroundColor(Color.rgb(16, 24, 32))
        }

        val scroll = ScrollView(this).apply {
            isFillViewport = true
            addView(page)
        }

        val brand = TextView(this).apply {
            text = "OKAPY"
            textSize = 34f
            setTextColor(Color.WHITE)
            typeface = Typeface.create("sans-serif-black", Typeface.BOLD)
            letterSpacing = 0.12f
        }
        page.addView(brand)

        val subtitle = TextView(this).apply {
            text = "GEOMETRY DASH LAUNCHER"
            textSize = 12f
            letterSpacing = 0.16f
            setTextColor(Color.rgb(112, 225, 193))
        }
        page.addView(subtitle, spaced(4, 28))

        val statusCard = card()
        val statusTitle = TextView(this).apply {
            text = "GAME STATUS"
            textSize = 11f
            letterSpacing = 0.12f
            setTextColor(Color.rgb(151, 169, 181))
        }
        statusCard.addView(statusTitle)
        gameStatus = TextView(this).apply {
            text = "Checking Geometry Dash…"
            textSize = 17f
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
        }
        statusCard.addView(gameStatus, spaced(0, 8))
        page.addView(statusCard, spaced(0, 26))

        val launchButton = Button(this).apply {
            text = "LAUNCH GEOMETRY DASH"
            textSize = 14f
            isAllCaps = false
            setTextColor(Color.rgb(16, 24, 32))
            background = rounded(Color.rgb(112, 225, 193), 16)
            setOnClickListener { launchGame() }
        }
        page.addView(launchButton, LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, dp(54)
        ))

        val refreshButton = Button(this).apply {
            text = "Check installation again"
            isAllCaps = false
            setTextColor(Color.WHITE)
            background = rounded(Color.rgb(39, 54, 66), 14)
            setOnClickListener { refreshGameStatus() }
        }
        page.addView(refreshButton, spaced(0, 10).apply { height = dp(48) })

        val infoCard = card()
        val infoTitle = TextView(this).apply {
            text = "ABOUT THIS BUILD"
            textSize = 11f
            letterSpacing = 0.12f
            setTextColor(Color.rgb(151, 169, 181))
        }
        infoCard.addView(infoTitle)
        val info = TextView(this).apply {
            text = "Okapy Launcher 0.1.0\nAndroid edition\n\nThis first build is a launcher foundation. It can check for and open the installed game. Mod loading is not available yet."
            textSize = 14f
            setTextColor(Color.rgb(225, 233, 238))
            setLineSpacing(dp(4).toFloat(), 1f)
        }
        infoCard.addView(info, spaced(0, 10))
        page.addView(infoCard, spaced(0, 22))

        diagnosticText = TextView(this).apply {
            textSize = 12f
            setTextColor(Color.rgb(151, 169, 181))
        }
        page.addView(diagnosticText, spaced(0, 16))

        setContentView(scroll)
    }

    private fun refreshGameStatus() {
        val installed = packageManager.getLaunchIntentForPackage(gamePackage) != null
        gameStatus.text = if (installed) {
            "Geometry Dash is installed"
        } else {
            "Geometry Dash was not detected"
        }
        diagnosticText.text = "Package check: $gamePackage\nLauncher status: ready"
    }

    private fun launchGame() {
        val intent = packageManager.getLaunchIntentForPackage(gamePackage)
        if (intent == null) {
            Toast.makeText(
                this,
                "Geometry Dash was not found. Install the official game first.",
                Toast.LENGTH_LONG
            ).show()
            refreshGameStatus()
            return
        }

        try {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(intent)
        } catch (_: ActivityNotFoundException) {
            Toast.makeText(this, "Android could not open Geometry Dash.", Toast.LENGTH_LONG).show()
        }
    }

    private fun card(): LinearLayout = LinearLayout(this).apply {
        orientation = LinearLayout.VERTICAL
        setPadding(dp(18), dp(18), dp(18), dp(18))
        background = rounded(Color.rgb(27, 40, 51), 18)
    }

    private fun rounded(color: Int, radius: Int) = GradientDrawable().apply {
        setColor(color)
        cornerRadius = dp(radius).toFloat()
    }

    private fun spaced(left: Int, top: Int) =
        LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        ).apply {
            if (left != 0 || top != 0) setMargins(dp(left), dp(top), 0, 0)
        }

    private fun dp(value: Int): Int =
        (value * resources.displayMetrics.density).toInt()
}
