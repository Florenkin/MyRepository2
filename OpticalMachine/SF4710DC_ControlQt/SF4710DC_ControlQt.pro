QT += core gui widgets serialport

CONFIG += c++11
TARGET = SF4710DC_ControlQt
TEMPLATE = app

win32-msvc:QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    src/main.cpp \
    src/DeviceController.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/DeviceController.h \
    src/MainWindow.h

INCLUDEPATH += $$PWD/third_party/SF_dlp/inc
DEPENDPATH += $$PWD/third_party/SF_dlp/inc

win32 {
    win32-msvc {
        SF_DLP_LIB_PATH = $$PWD/third_party/SF_dlp/lib_win/MSVC2015_64
        LIBS += -L$$SF_DLP_LIB_PATH -ldlphandle
        QMAKE_POST_LINK += $$quote(cmd /c copy /Y "$$replace(SF_DLP_LIB_PATH, /, \\)\\dlphandle.dll" "$$replace(OUT_PWD, /, \\)\\")
    }

    win32-g++ {
        SF_DLP_LIB_PATH = $$PWD/third_party/SF_dlp/lib_win/MinGW_32
        LIBS += -L$$SF_DLP_LIB_PATH -ldlphandle
        QMAKE_POST_LINK += $$quote(cmd /c copy /Y "$$replace(SF_DLP_LIB_PATH, /, \\)\\dlphandle.dll" "$$replace(OUT_PWD, /, \\)\\")
    }
}
