CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Anonymizer)

FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})

SET(QT_USE_QTXML TRUE)

SET(Anonymizer_SOURCES main.cpp mainwindow.cpp xmlTools.cpp config.cpp 
	tools.cpp askDialog.cpp imageEditor.cpp qImageLabel.cpp askPatientInfo.cpp 
	askSomething.cpp anonymizer.cpp scanning.cpp copy.cpp
	patient.cpp image.cpp study.cpp series.cpp extract.cpp file.cpp property.cpp)
SET(Anonymizer_HEADERS mainwindow.h askDialog.h imageEditor.h qImageLabel.h askPatientInfo.h
	askSomething.h anonymizer.h anonymization.h scanning.h copy.h boundingBox.h)
SET(Anonymizer_FORMS mainwindow.ui askPatientInfo.ui)


QT4_WRAP_CPP(Anonymizer_HEADERS_MOC ${Anonymizer_HEADERS})
QT4_WRAP_UI(Anonymizer_FORMS_HEADERS ${Anonymizer_FORMS})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

INCLUDE_DIRECTORIES(
	#DCMTK
	C:/pro/dcmtk/build/include
	C:/pro/dcmtk/src/dcmdata/include/dcmtk/dcmdata 
	C:/pro/dcmtk/build/include
	C:/pro/dcmtk/src/dcmdata/include
	C:/pro/dcmtk/src/ofstd/include
	C:/pro/dcmtk/src/oflog/include
	C:/pro/dcmtk/src/dcmimage/include/dcmtk
	C:/pro/dcmtk/src/dcmimgle/include
	C:/pro/dcmtk/src/dcmimgle/include/dcmtk/dcmimgle
	C:/pro/dcmtk/src/dcmjpeg/include
	C:/pro/dcmtk/src/dcmjpeg/include/dcmtk/dcmjpeg
	C:/pro/dcmtk/src/dcmimage/include
	C:/pro/dcmtk/src/dcmimage/include/dcmtk/dcmimage
	#OpenCV
	C:/pro/OpenCV/build/include
	C:/pro/OpenCV/src/modules/highgui/include
	C:/pro/OpenCV/src/modules/core/include
	C:/pro/OpenCV/src/modules/imgproc/include
	C:/pro/OpenCV/src/modules/photo/include
	C:/pro/OpenCV/src/modules/video/include
	C:/pro/OpenCV/src/modules/features2d/include
	C:/pro/OpenCV/src/modules/calib3d/include
	C:/pro/OpenCV/src/modules/contrib/include
	C:/pro/OpenCV/src/modules/flann/include
	C:/pro/OpenCV/src/modules/gpu/include
	C:/pro/OpenCV/src/modules/gpu/include
	C:/pro/OpenCV/src/modules/highgui/include
	C:/pro/OpenCV/src/modules/imgproc/include
	C:/pro/OpenCV/src/modules/legacy/include
	C:/pro/OpenCV/src/modules/ml/include
	C:/pro/OpenCV/src/modules/objdetect/include
)
 
 
LINK_DIRECTORIES(C:/pro/dcmtk/lib/$(ConfigurationName))
LINK_DIRECTORIES(C:/pro/OpenCV/build/x86/vc10/lib C:/pro/OpenCV/build/x86/vc10/bin)
LINK_DIRECTORIES(C:/pro/ffmpeg/lib)

ADD_EXECUTABLE(Anonymizer
    ${Anonymizer_SOURCES}
    ${Anonymizer_HEADERS_MOC}
    ${Anonymizer_FORMS_HEADERS}
)

TARGET_LINK_LIBRARIES(Anonymizer ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(Anonymizer ${QT_QTXMLPATTERNS_LIBRARY})
TARGET_LINK_LIBRARIES(Anonymizer
	netapi32.lib 
	wsock32.lib
#dcmtk
	ofstd.lib
	dcmdata.lib
	dcmimgle.lib
	dcmimage.lib
	ijg8.lib
	ijg12.lib
	ijg16.lib
	libi2d.lib
	dcmjpeg.lib
	dcmnet.lib
	dcmdsig.lib
	dcmsr.lib
	dcmpstat.lib
	dcmtls.lib
	dcmwlm.lib
	dcmjpls.lib
	dcmqrdb.lib
	oflog.lib
	charls.lib
#ffmpeg
	avcodec.lib
	avdevice.lib
	avfilter.lib
	avformat.lib
	avutil.lib
	postproc.lib
	swresample.lib
	swscale.lib
#OpenCV
	debug OpenCV_core249d.lib
	debug OpenCV_imgproc249d.lib
	debug OpenCV_highgui249d.lib
	debug OpenCV_video249d.lib
	optimized OpenCV_core249.lib
	optimized OpenCV_imgproc249.lib
	optimized OpenCV_highgui249.lib
	optimized OpenCV_video249.lib
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${KDE4_INCLUDES} ${QT_INCLUDES})
