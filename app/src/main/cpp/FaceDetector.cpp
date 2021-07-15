#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/imgproc/types_c.h>

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

using namespace cv;
using namespace std;

CascadeClassifier cascadeClassifier;


extern "C"
JNIEXPORT void JNICALL
Java_com_jansir_opencvdemo_FaceDetector_loadCascade(JNIEnv *env, jobject thiz, jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    cascadeClassifier.load(filePath) ;
    env->ReleaseStringUTFChars(filePath_,filePath);
    LOGE("人脸识别级联分类器加载成功");
}


extern "C" {
JNIEXPORT int
JNICALL
Java_com_jansir_opencvdemo_FaceDetector_faceDetectSave(JNIEnv *env, jobject thiz,
                                                               jobject bitmap);
// bitmap -> mat
Mat bitmap2Mat(jobject pJobject);
// mat -> bitmap
void mat2bitmap(JNIEnv *env, Mat mat, jobject bitmap);
}

//实现bitmap2Mat方法
Mat bitmap2Mat(JNIEnv *env , jobject bitmap){
    AndroidBitmapInfo info;
    //获取bitmap的信息
    AndroidBitmap_getInfo(env, bitmap, &info);
    //定义一个可以接受任何类型的指针
    void *pixels;
    // 将 pixels 指向图片的像素地址。
    // 这个方法会锁定图片的所有像素不能改变，直到调用AndroidBitmap_unlockPixels 方法
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    Mat mat;
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGD("nMatToBitmap: CV_8UC4 -> RGBA_8888");
        mat = Mat(info.height, info.width, CV_8UC4, pixels);
    } else if (info.format = ANDROID_BITMAP_FORMAT_RGB_565) {
        LOGD("nMatToBitmap: CV_8UC2 -> RGBA_565");
        mat = Mat(info.height, info.width, CV_8UC2, pixels);
    }
    // 解除对图片像素的锁定
    AndroidBitmap_unlockPixels(env, bitmap);
    return mat;
}

void mat2bitmap(JNIEnv *env , Mat src , jobject bitmap){
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        Mat tmp(info.height, info.width, CV_8UC4, pixels);
        if (src.type() == CV_8UC1) {
            LOGD("nMatToBitmap: CV_8UC1 -> RGBA_8888");
            cvtColor(src, tmp, COLOR_GRAY2RGBA);
        } else if (src.type() == CV_8UC3) {
            LOGD("nMatToBitmap: CV_8UC3 -> RGBA_8888");
            cvtColor(src, tmp, COLOR_RGB2RGBA);
        } else if (src.type() == CV_8UC4) {
            LOGD("nMatToBitmap: CV_8UC4 -> RGBA_8888");
            src.copyTo(tmp);
        }
    } else {
        // info.format == ANDROID_BITMAP_FORMAT_RGB_565
        Mat tmp(info.height, info.width, CV_8UC2, pixels);
        if (src.type() == CV_8UC1) {
            LOGD("nMatToBitmap: CV_8UC1 -> RGB_565");
            cvtColor(src, tmp, COLOR_GRAY2BGR565);
        } else if (src.type() == CV_8UC3) {
            LOGD("nMatToBitmap: CV_8UC3 -> RGB_565");
            cvtColor(src, tmp, COLOR_RGB2BGR565);
        } else if (src.type() == CV_8UC4) {
            LOGD("nMatToBitmap: CV_8UC4 -> RGB_565");
            cvtColor(src, tmp, COLOR_RGBA2BGR565);
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT jint JNICALL
Java_com_jansir_opencvdemo_FaceDetector_faceDetectSave(JNIEnv *env, jobject thiz, jobject bitmap) {
     // opencv 操作图片操作的都是 矩阵 Mat
     // 1. bitmap2Mat
     Mat mat = bitmap2Mat(env, bitmap);
     Mat grayMat;
     // 2. 转成灰度图，提升运算速度，灰度图所对应的 CV_8UC1 单颜色通道，信息量少 0-255 1u
     cvtColor(mat, grayMat, CV_RGBA2GRAY);
     // 3. 转换直方图均衡化补偿
     Mat equalizeMat;
     equalizeHist(grayMat, equalizeMat);
     // 4. 检测人脸
     vector<Rect> faces;
     cascadeClassifier.detectMultiScale(equalizeMat, faces, 1.1, 5, 2 ,
                                        Size(50, 50));
     LOGE("检测到人脸的个数：%d", faces.size());
     if (faces.size() == 1) {
         Rect faceRect = faces[0];
         // 画一个框框，标记出人脸
         rectangle(mat, faceRect, Scalar(255, 155, 155), 3);
         mat2bitmap(env, mat, bitmap);
         // 只裁剪人脸部分的直方均衡补偿
         Mat saveMat = Mat(equalizeMat, faceRect);
         // 保存图片
         imwrite("/sdcard/face_detect_result.png", equalizeMat);
         return 1;
     }
     return 0;

}