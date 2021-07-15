package com.jansir.opencvdemo

import android.Manifest
import android.content.pm.PackageManager.PERMISSION_GRANTED
import android.graphics.drawable.BitmapDrawable
import android.os.Bundle
import android.widget.Button
import android.widget.ImageView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.hjq.permissions.OnPermissionCallback
import com.hjq.permissions.Permission
import com.hjq.permissions.XXPermissions


class MainActivity : AppCompatActivity() {


    private val faceDetector by lazy {
        FaceDetector()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        XXPermissions.with(this)
            .permission(Permission.Group.STORAGE)
            .request { permissions, all ->
                onGetPermission()
            }

    }

    private fun onGetPermission() {
        faceDetector.init(this@MainActivity)
        val iv = findViewById<ImageView>(R.id.iv1)
        findViewById<Button>(R.id.btnFaceDetect).setOnClickListener {
            val result = faceDetector.faceDetectSave((iv.drawable as BitmapDrawable).bitmap)
            findViewById<TextView>(R.id.tvDetectResult).text = "人脸识别个数 : $result"
        }
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}