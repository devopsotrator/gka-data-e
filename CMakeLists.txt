cmake_minimum_required(VERSION 3.7)
project(gka_data_e)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(CMAKE_CXX_STANDARD 14)

find_package(Eina REQUIRED)
find_package(Eo REQUIRED)
find_package(Eolian REQUIRED)
find_package(Ecore REQUIRED)
find_package(Evas REQUIRED)
find_package(Efl REQUIRED)
find_package(Elementary REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1" -DPACKAGE_DATA_DIR="${CMAKE_BINARY_DIR}")

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

add_subdirectory(images)
add_subdirectory(data_src)
add_subdirectory(data_lib)
add_subdirectory(data_lib_tests)
