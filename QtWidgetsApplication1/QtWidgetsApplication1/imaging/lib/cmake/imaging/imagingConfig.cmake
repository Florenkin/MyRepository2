
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was imagingConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# find_dependency
include(CMakeFindDependencyMacro)

# find Qt5 dependency (Core is used, not Widgets based on CMakeLists.txt)
find_dependency(Qt5 REQUIRED COMPONENTS Core)

# find OpenCV dependency
find_dependency(OpenCV REQUIRED COMPONENTS core features2d calib3d imgproc imgcodecs)

# find PCL dependency (conditional, based on WITH_PCL option)
# Note: PCL is optional in build but required if WITH_PCL was ON
# You may need to make this optional based on how the library was built
find_dependency(PCL 1.8 REQUIRED COMPONENTS common octree io segmentation)

# find sub-library dependencies
find_dependency(globaldata REQUIRED)
find_dependency(cameras REQUIRED)
find_dependency(reconstruction3d REQUIRED)

# Optional motioncontrol dependency
# If motioncontrol was enabled during build, uncomment the following:
# find_dependency(motioncontrol REQUIRED)

# Include targets - note the namespace matches CMakeLists.txt
# The export uses ${PROJECT_NAME}Targets.cmake which is imagingTargets.cmake
include("${CMAKE_CURRENT_LIST_DIR}/imagingTargets.cmake")

# Check required components (if using COMPONENTS feature)
# check_required_components(imaging)

# Set version (matching PROJECT_VERSION from CMakeLists.txt)
set(imaging_VERSION 1.0.0)

# Define include directory variable (matches CMakeLists.txt)
get_target_property(IMAGING_INCLUDE_DIR imaging::imaging INTERFACE_INCLUDE_DIRECTORIES)
if(NOT IMAGING_INCLUDE_DIR)
    set(IMAGING_INCLUDE_DIR )
endif()

# Add definitions (Optional - similar to commented section in original)
# target_compile_definitions(imaging::imaging INTERFACE IMAGING_FOUND)

# Additional helper variables (matches CMakeLists.txt)
set(IMAGING_VERSION_MAJOR 1)
set(IMAGING_VERSION_MINOR 0)
set(IMAGING_VERSION_PATCH 0)
