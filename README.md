# OpenCvFaceDetectDemo
openCv 实现人脸识别

OpenCV是一个基于BSD许可（开源）发行的跨平台计算机视觉和机器学习软件库，可以运行在Linux、Windows、Android和Mac OS操作系统上。  它轻量级而且高效——由一系列 C 函数和少量 C++ 类构成，同时提供了Python、Ruby、MATLAB等语言的接口，实现了图像处理和计算机视觉方面的很多通用算法。   

demo实现效果图:
![image.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f67f90562dd840c585d2d4b36c52e2bb~tplv-k3u1fbpfcp-watermark.image)

步骤: 
##### 1.新建一个包含ndk的android项目
 
![image.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c8a9e8a1ec0642149dad6de80a858fed~tplv-k3u1fbpfcp-watermark.image)
##### 2. 下载好opencv-android-sdk,并将相关文件复制到项目中 
下载地址 : [https://sourceforge.net/projects/opencvlibrary/files/](https://sourceforge.net/projects/opencvlibrary/files/)  
 1.将jni目录下的include 头文件复制到cpp目录下  
 2.将libs目录下的so文件复制到main/jniLibs下
 

![image.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5d55c2227cc04666b35c77e8283f3ba4~tplv-k3u1fbpfcp-watermark.image)
头文件和so文件复制好的工程目录如下:

![image.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e08f3024618d491984cb2bd6c5661b17~tplv-k3u1fbpfcp-watermark.image)  
##### 3.修改app build.gradle文件
```java
externalNativeBuild {
            cmake {
                cppFlags("-std=c++11", "-frtti", "-fexceptions -lz")
                // 如果需要模拟器上运行 需要添加x86 架构
                abiFilters 'arm64-v8a',"armeabi-v7a"
                arguments "-DANDROID_STL=c++_shared"
            }
        }
//配置文件目录     
sourceSets {
      main {
          jniLibs.srcDirs = ['src/main/jniLibs']
      }
  }
//配置ndk版本
ndkVersion '22.1.7171670'
```
##### 4.修改CMakeLists.txt文件
```c
#cmake最低版本
cmake_minimum_required(VERSION 3.10.2)
#项目名字
project("opencvdemo")

#把CMAKE_SOURCE_DIR目录的下的所有 .cpp 和 .c 文件赋值给 SRC_LIST
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR} SRC_LIST)

# 输出SRC_LIST
MESSAGE("SRC_LIST= " ${SRC_LIST})
# 输出打印构建目录
MESSAGE("This is HELLO_BINARY_DIR " ${HELLO_BINARY_DIR})
# 输出打印资源目录
MESSAGE("This is HELLO_SOURCE_DIR " ${HELLO_SOURCE_DIR})
# 输出打印资源目录，与HELLO_SOURCE_DIR 一样
MESSAGE("This is PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})
# 输出打印 CMake 资源目录，与 PROJECT_SOURCE_DIR 一样
MESSAGE(STATUS "This is CMAKE_SOURCE_DIR " ${CMAKE_SOURCE_DIR})

#  将SRC_LIST编译成native-lib.so
add_library(native-lib SHARED ${SRC_LIST})

#导入头文件
include_directories(include)

#导入main/jniLibs下的so库文件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}")

find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

#链接动态库
target_link_libraries(
        native-lib
        android
        opencv_java4
        jnigraphics
        ${log-lib} )
```
##### 5.编写c++代码
FaceDetector.cpp  

加载人脸识别级联分类器  
```c
extern "C"
JNIEXPORT void JNICALL
Java_com_jansir_opencvdemo_FaceDetector_loadCascade(JNIEnv *env, jobject thiz, jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    cascadeClassifier.load(filePath) ;
    env->ReleaseStringUTFChars(filePath_,filePath);
    LOGE("人脸识别级联分类器加载成功");
}
```

进行人脸识别并将图片保存到sdcard中,然后返回识别结果
```c
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
```

kotlin代码
```kotlin

 private val faceDetector by lazy {
        FaceDetector()
    }


 faceDetector.init(this@MainActivity)
 val iv = findViewById<ImageView>(R.id.iv1)
 findViewById<Button>(R.id.btnFaceDetect).setOnClickListener {
     val result = faceDetector.faceDetectSave((iv.drawable as BitmapDrawable).bitmap)
     findViewById<TextView>(R.id.tvDetectResult).text = "人脸识别个数 : $result"
 }
```

demo 地址 :[ https://github.com/jiangpana/OpenCvFaceDetectDemo](https://github.com/jiangpana/OpenCvFaceDetectDemo)   
