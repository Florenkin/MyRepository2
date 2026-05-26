cmake_minimum_required(VERSION 3.16)

project(PointCloudViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(PCL_ROOT "D:/library/PCL 1.10.0" CACHE PATH "PCL installation directory")
list(APPEND CMAKE_PREFIX_PATH "${PCL_ROOT}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PCL_ROOT}/3rdParty/VTK/lib/cmake/vtk-8.2")
list(APPEND CMAKE_PREFIX_PATH "${PCL_ROOT}/3rdParty/Boost/lib/cmake/Boost-1.72.0")

find_package(Qt5 REQUIRED COMPONENTS Widgets)

# The bundled VTK 8.2 package shipped with this PCL distribution references
# these Debug import targets without declaring them in VTKTargets.cmake.
if(WIN32)
    if(NOT TARGET vtkGUISupportMFC)
        add_library(vtkGUISupportMFC SHARED IMPORTED GLOBAL)
    endif()
    if(NOT TARGET vtkRenderingExternal)
        add_library(vtkRenderingExternal SHARED IMPORTED GLOBAL)
    endif()
endif()

find_package(PCL 1.10 REQUIRED COMPONENTS common filters io visualization)
find_package(VTK REQUIRED)

add_executable(PointCloudViewer WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/PointCloudLoader.cpp
    src/PointCloudLoader.h
    src/PointCloudView.cpp
    src/PointCloudView.h
)

set(QT_RUNTIME_DIR "C:/Qt/Qt5.15.2/5.15.2/msvc2019_64/bin" CACHE PATH "Qt runtime DLL directory")
set(PCL_RUNTIME_DIR "${PCL_ROOT}/bin" CACHE PATH "PCL runtime DLL directory")
set(PCL_VTK_RUNTIME_DIR "${PCL_ROOT}/3rdParty/VTK/bin" CACHE PATH "VTK runtime DLL directory")
set(PCL_OPENNI_RUNTIME_DIR "${PCL_ROOT}/3rdParty/OpenNI2/Redist" CACHE PATH "OpenNI2 runtime DLL directory")

set_target_properties(PointCloudViewer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_RUNTIME_DIR};${PCL_RUNTIME_DIR};${PCL_VTK_RUNTIME_DIR};${PCL_OPENNI_RUNTIME_DIR};%PATH%"
)

target_include_directories(PointCloudViewer PRIVATE
    src
    ${PCL_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
)

target_compile_definitions(PointCloudViewer PRIVATE
    ${PCL_DEFINITIONS}
)

if(MSVC)
    target_compile_options(PointCloudViewer PRIVATE /utf-8)
endif()

target_link_libraries(PointCloudViewer PRIVATE
    Qt5::Widgets
    ${PCL_LIBRARIES}
    ${VTK_LIBRARIES}
    vtkGUISupportQt
    vtkGUISupportQtOpenGL
    vtkRenderingQt
)

if(VTK_VERSION VERSION_LESS "8.90.0")
    include(${VTK_USE_FILE})
else()
    vtk_module_autoinit(
        TARGETS PointCloudViewer
        MODULES ${VTK_LIBRARIES}
    )
endif()
