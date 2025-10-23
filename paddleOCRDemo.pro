#-------------------------------------------------
#
# Project created by QtCreator 2021-02-08T21:17:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = paddleOCRDemo
TEMPLATE = app
CONFIG += c++17
DESTDIR=$$OUT_PWD

DEPEN=$$PWD/inference

DES=$$DESTDIR/inference
QMAKE_PRE_LINK += $(COPY_DIR) $$shell_path($$DEPEN) $$shell_path($$DES) \
    && $(COPY) $$shell_path($$PWD/config.txt) $$shell_path($$OUT_PWD/config.txt) \
    && $(COPY) $$shell_path($$PWD/ppocr_keys_v1.txt) $$shell_path($$OUT_PWD/ppocr_keys_v1.txt)

# PADDLE_PATH = $$PWD/paddle_inference #use your own path of paddle_inference
win32: LIBS += -L$$PWD/paddle_inference/paddle/lib/ -lpaddle_inference -lcommon
INCLUDEPATH += $$PWD/paddle_inference/paddle/include
DEPENDPATH += $$PWD/paddle_inference/paddle/include

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/cryptopp/lib/ -lcryptopp-static
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/cryptopp/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/cryptopp/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/cryptopp/lib/cryptopp-static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/cryptopp/lib/libcryptopp-static.a

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/gflags/lib/ -lgflags_static
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/gflags/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/gflags/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/gflags/lib/gflags_static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/gflags/lib/libgflags_static.a

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/glog/lib/ -lglog
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/glog/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/glog/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/glog/lib/glog.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/glog/lib/libglog.a

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/mkldnn/lib/ -lmkldnn
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/mkldnn/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/mkldnn/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/paddle_inference/third_party/install/mklml/lib/ -lmklml -llibiomp5md
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/mklml/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/mklml/include


win32: LIBS += -L$$PWD/paddle_inference/third_party/install/protobuf/lib/ -llibprotobuf
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/protobuf/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/protobuf/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/protobuf/lib/libprotobuf.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/protobuf/lib/liblibprotobuf.a

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/utf8proc/lib/ -lutf8proc_static
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/utf8proc/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/utf8proc/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/utf8proc/lib/utf8proc_static.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/utf8proc/lib/libutf8proc_static.a

win32: LIBS += -L$$PWD/paddle_inference/third_party/install/xxhash/lib/ -lxxhash
INCLUDEPATH += $$PWD/paddle_inference/third_party/install/xxhash/include
DEPENDPATH += $$PWD/paddle_inference/third_party/install/xxhash/include
win32:!win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/xxhash/lib/xxhash.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/paddle_inference/third_party/install/xxhash/lib/libxxhash.a

INCLUDEPATH += E:/github/opencv/opencv/build/install/include \
    $$PADDLE_PATH/paddle/include \
    $$PADDLE_PATH/third_party/install/cryptopp/include \
    $$PADDLE_PATH/third_party/install/glog/include \
    $$PADDLE_PATH/third_party/install/mkldnn/include \
    $$PADDLE_PATH/third_party/install/mklml/include \
    $$PADDLE_PATH/third_party/install/protobuf/include \
    $$PADDLE_PATH/third_party/install/xxhash/include
# paddle_inference


#you may need to modify the version of opencv

win32:CONFIG(release, debug|release): LIBS += -LD:/env/opencv/build/x64/vc16/lib/ -lopencv_world4100
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/env/opencv/build/x64/vc16/lib/ -lopencv_world4100d
INCLUDEPATH += D:/env/opencv/build/include
DEPENDPATH += D:/env/opencv/build/include



# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    paddle/clipper.cpp \
    paddle/config.cpp \
    paddle/ocr_cls.cpp \
    paddle/ocr_det.cpp \
    paddle/ocr_rec.cpp \
    paddle/postprocess_op.cpp \
    paddle/preprocess_op.cpp \
    paddle/utility.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
        mainwindow.h \
    paddle/clipper.h \
    paddle/config.h \
    paddle/ocr_cls.h \
    paddle/ocr_det.h \
    paddle/ocr_rec.h \
    paddle/postprocess_op.h \
    paddle/preprocess_op.h \
    paddle/utility.h \
    mainwindow.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    resource.qrc

RC_ICONS = resource/XD-OCR.ico
