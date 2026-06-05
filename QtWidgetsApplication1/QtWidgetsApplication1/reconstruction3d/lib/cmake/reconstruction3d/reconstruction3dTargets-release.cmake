#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "reconstruction3d::reconstruction3d" for configuration "Release"
set_property(TARGET reconstruction3d::reconstruction3d APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(reconstruction3d::reconstruction3d PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/win/x64/reconstruction3d.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/win/x64/reconstruction3d.dll"
  )

list(APPEND _cmake_import_check_targets reconstruction3d::reconstruction3d )
list(APPEND _cmake_import_check_files_for_reconstruction3d::reconstruction3d "${_IMPORT_PREFIX}/lib/win/x64/reconstruction3d.lib" "${_IMPORT_PREFIX}/bin/win/x64/reconstruction3d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
