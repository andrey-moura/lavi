#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "lavi-lang::lavi-lang" for configuration "Debug"
set_property(TARGET lavi-lang::lavi-lang APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(lavi-lang::lavi-lang PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/liblavi-lang.a"
  )

list(APPEND _cmake_import_check_targets lavi-lang::lavi-lang )
list(APPEND _cmake_import_check_files_for_lavi-lang::lavi-lang "${_IMPORT_PREFIX}/lib/liblavi-lang.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
