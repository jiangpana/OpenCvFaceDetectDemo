package com.jansir.opencvdemo

import android.content.Context
import android.graphics.Bitmap
import java.io.File
import java.io.FileOutputStream
import java.io.IOException

class FaceDetector {

    fun init(context: Context) {
        val path = copyCascadeToLocal(context)
        loadCascade(path)
    }

    /**
     *  将lbpcascade_frontalface文件复制到本地
     */
    private fun copyCascadeToLocal(context: Context): String {
        val ris = context.resources.openRawResource(R.raw.lbpcascade_frontalface)
        val cascadeDir = context.getDir("cascade", Context.MODE_PRIVATE)
        val cascadeFile = File(cascadeDir, "lbpcascade_frontalface.xml")
        val os = FileOutputStream(cascadeFile)
        val buffer = ByteArray(4 * 1024)
        var bytesRead: Int
        try {
            while (ris.read(buffer).also {
                    bytesRead = it
                } != -1) {
                os.write(buffer, 0, bytesRead)
            }
            ris.close()
            os.close()
        } catch (e: IOException) {
            e.printStackTrace()
        }
        return cascadeFile.getAbsolutePath();
    }

    external fun loadCascade(path: String)
    external fun faceDetectSave(bitmap: Bitmap): Int
}