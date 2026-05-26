QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cintvalidator.cpp \
    main.cpp \
    mainwidget.cpp \
    mycombobox.cpp \
    countdowndialog.cpp

HEADERS += \
    cintvalidator.h \
    mainwidget.h \
    mycombobox.h \
    tabledelegates.h \
    countdowndialog.h

FORMS += \
    mainwidget.ui \
    countdowndialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32 {
    # MSVC编译器配置
    win32-msvc {
        # 设定MSVC库路径
        SF_DLP_LIB_PATH = $$PWD/SF_dlp/lib_win/MSVC2015_64
        # 增加路径有效性检查
        !exists($$SF_DLP_LIB_PATH) {
            error("MSVC库路径不存在: $$SF_DLP_LIB_PATH")
        }
        # Release模式
        CONFIG(release, debug|release) {
            LIBS += -L$$SF_DLP_LIB_PATH -ldlphandle
        }
        # Debug模式
        else:CONFIG(debug, debug|release) {
            LIBS += -L$$SF_DLP_LIB_PATH -ldlphandled
        }
        message("使用MSVC编译器，链接库路径: $$SF_DLP_LIB_PATH")
    }

    # MinGW编译器配置
    win32-g++ {
        # 假设定MinGW库路径（假设目录结构类似）
        SF_DLP_LIB_PATH = $$PWD/SF_dlp/lib_win/MinGW_32
        # 增加路径有效性检查
        !exists($$SF_DLP_LIB_PATH) {
            error("MSVC库路径不存在: $$SF_DLP_LIB_PATH")
        }
        # Release模式
        CONFIG(release, debug|release) {
            LIBS += -L$$SF_DLP_LIB_PATH -ldlphandle
        }
        # Debug模式
        else:CONFIG(debug, debug|release) {
            LIBS += -L$$SF_DLP_LIB_PATH -ldlphandled
        }
        message("使用MinGW编译器，链接库路径: $$SF_DLP_LIB_PATH")
    }
}


INCLUDEPATH += $$PWD/SF_dlp/inc
DEPENDPATH += $$PWD/SF_dlp/inc
