#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "flatbuffers::flatbuffers_shared" for configuration ""
set_property(TARGET flatbuffers::flatbuffers_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(flatbuffers::flatbuffers_shared PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libflatbuffers.so.1.8.0"
  IMPORTED_SONAME_NOCONFIG "libflatbuffers.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS flatbuffers::flatbuffers_shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_flatbuffers::flatbuffers_shared "${_IMPORT_PREFIX}/lib/libflatbuffers.so.1.8.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
