cmake_minimum_required(VERSION 3.10) # CMake install : https://cmake.org/download/
project(cmake_test LANGUAGES CXX)

#设置为Release
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/WorkDocument/Qt/5.13.2/msvc2015_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt5 COMPONENTS Widgets SerialPort REQUIRED) # Qt COMPONENTS
aux_source_directory(./src srcs)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(DLP4710_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SF_dlp/lib_win/MSVC2015_64)
set(DLP4710_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SF_dlp/inc)
set(DLP4710_LIBS "${DLP4710_LIBS_DIR}/dlphandle.lib")
file(COPY ${DLP4710_LIBS_DIR}/dlphandle.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})

include_directories(
    ${DLP4710_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs} 
) 
target_link_libraries(${PROJECT_NAME} PRIVATE 
                    Qt5::Widgets 
                    Qt5::SerialPort
                    ${DLP4710_LIBS}) # Qt5 Shared Library

set_target_properties(${PROJECT_NAME} PROPERTIES 
                WIN32_EXECUTABLE OFF
)