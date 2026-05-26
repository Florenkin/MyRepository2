cmake_minimum_required(VERSION 3.16)

project(SF4710DC_ControlQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort)
find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt
    HINTS
        "${Qt5_DIR}/../../../bin"
        "${QTDIR}/bin"
)

if(NOT WINDEPLOYQT_EXECUTABLE)
    message(FATAL_ERROR "Could not find windeployqt.exe. Make sure the Qt5 kit is installed and Qt5_DIR/QTDIR points to it.")
endif()

set(SF4710SDK_ROOT ""
    CACHE PATH "Path to the SF4710 SDK root that contains SF_dlp")

if(NOT SF4710SDK_ROOT)
    set(SF4710SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/third_party")
endif()

if(NOT EXISTS "${SF4710SDK_ROOT}/SF_dlp/inc/dlphandle.h")
    message(FATAL_ERROR "Set -DSF4710SDK_ROOT to a directory that contains SF_dlp")
endif()

set(SF4710SDK_INCLUDE_DIR "${SF4710SDK_ROOT}/SF_dlp/inc")

if(MSVC)
    set(SF4710SDK_LIB_DIR "${SF4710SDK_ROOT}/SF_dlp/lib_win/MSVC2015_64")
    set(SF4710SDK_RELEASE_LIB "${SF4710SDK_LIB_DIR}/dlphandle.lib")
    set(SF4710SDK_DEBUG_LIB "${SF4710SDK_LIB_DIR}/dlphandled.lib")
    set(SF4710SDK_RELEASE_DLL "${SF4710SDK_LIB_DIR}/dlphandle.dll")
    set(SF4710SDK_DEBUG_DLL "${SF4710SDK_LIB_DIR}/dlphandled.dll")
elseif(MINGW)
    set(SF4710SDK_LIB_DIR "${SF4710SDK_ROOT}/SF_dlp/lib_win/MinGW_32")
    set(SF4710SDK_RELEASE_LIB "${SF4710SDK_LIB_DIR}/libdlphandle.a")
    set(SF4710SDK_DEBUG_LIB "${SF4710SDK_LIB_DIR}/libdlphandled.a")
    set(SF4710SDK_RELEASE_DLL "${SF4710SDK_LIB_DIR}/dlphandle.dll")
    set(SF4710SDK_DEBUG_DLL "${SF4710SDK_LIB_DIR}/dlphandled.dll")
else()
    message(FATAL_ERROR "This project is configured for Windows Qt5 builds with MSVC or MinGW.")
endif()

foreach(required_path
        "${SF4710SDK_INCLUDE_DIR}"
        "${SF4710SDK_RELEASE_LIB}"
        "${SF4710SDK_DEBUG_LIB}"
        "${SF4710SDK_RELEASE_DLL}"
        "${SF4710SDK_DEBUG_DLL}")
    if(NOT EXISTS "${required_path}")
        message(FATAL_ERROR "Required SF4710 SDK file or directory not found: ${required_path}")
    endif()
endforeach()

add_executable(SF4710DC_ControlQt WIN32
    src/main.cpp
    src/DeviceController.cpp
    src/DeviceController.h
    src/MainWindow.cpp
    src/MainWindow.h
)

target_include_directories(SF4710DC_ControlQt PRIVATE
    "${SF4710SDK_INCLUDE_DIR}"
)

target_link_libraries(SF4710DC_ControlQt PRIVATE
    Qt5::Widgets
    Qt5::SerialPort
    "${SF4710SDK_RELEASE_LIB}"
)

if(MSVC)
    target_compile_options(SF4710DC_ControlQt PRIVATE /utf-8)
endif()

add_custom_command(TARGET SF4710DC_ControlQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SF4710SDK_RELEASE_DLL}"
        "$<TARGET_FILE_DIR:SF4710DC_ControlQt>"
    COMMENT "Copying SF4710 DLP SDK runtime DLL")

add_custom_command(TARGET SF4710DC_ControlQt POST_BUILD
    COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --compiler-runtime
            "$<TARGET_FILE:SF4710DC_ControlQt>"
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:SF4710DC_ControlQt>"
    COMMENT "Deploying Qt runtime with windeployqt")
