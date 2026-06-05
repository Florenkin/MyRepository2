
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was reconstruction3dConfig.cmake.in                            ########

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

# Find PCL dependency (required if WITH_PCL was ON)
# Note: PCL requires Eigen, Flann, Boost, etc.
if(ON)
    # Set PCL_DIR if needed (user may need to set this)
    find_dependency(PCL 1.8 REQUIRED COMPONENTS common octree io)
endif()

# Find OpenCV dependency (required)
# Note: OpenCV 4.x is used in the project
find_dependency(OpenCV REQUIRED COMPONENTS core features2d imgproc calib3d flann imgcodecs)

# Find CUDA dependency (optional, only if built with CUDA)
if(OFF)
    find_dependency(CUDAToolkit REQUIRED)
endif()

# Include targets
include("${CMAKE_CURRENT_LIST_DIR}/reconstruction3dTargets.cmake")

# check required components
check_required_components(reconstruction3d)

# set version
set(reconstruction3d_VERSION 1.0.0)

# Add interface compile definitions
# Uncomment if you want to propagate a definition to consumers
# target_compile_definitions(reconstruction3d::reconstruction3d INTERFACE RECONSTRUCTION3D_FOUND)

# Optionally, provide a helper variable for include directories
get_target_property(reconstruction3d_INCLUDE_DIRS reconstruction3d::reconstruction3d INTERFACE_INCLUDE_DIRECTORIES)
