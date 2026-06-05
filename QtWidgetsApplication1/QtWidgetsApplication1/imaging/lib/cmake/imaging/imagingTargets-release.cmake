#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "imaging::imaging" for configuration "Release"
set_property(TARGET imaging::imaging APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(imaging::imaging PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/win/x64/imaging.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/win/x64/imaging.dll"
  )

list(APPEND _cmake_import_check_targets imaging::imaging )
list(APPEND _cmake_import_check_files_for_imaging::imaging "${_IMPORT_PREFIX}/lib/win/x64/imaging.lib" "${_IMPORT_PREFIX}/bin/win/x64/imaging.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
