cmake_minimum_required(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(focus_test)
message(STATUS "**********************Start to compile project ${PROJECT_NAME}**********************")

file(GLOB _SOURCES "*.c" "*.cpp")
file(GLOB _HEADERS "*.h")

find_package(Qt5 REQUIRED COMPONENTS Widgets Core QUIET)
# Generate the UI files
qt5_wrap_ui(outfiles mainwindow.ui)
add_executable(${PROJECT_NAME} ${_SOURCES} ${_HEADERS} ${outfiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core)
message(STATUS "Qt5 found at ${Qt5_DIR} for ${PROJECT_NAME}")

# ---[ Reconstruction3D]
target_link_libraries(${PROJECT_NAME} PRIVATE reconstruction3d)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${EXAMPLE_DIR}"
)
