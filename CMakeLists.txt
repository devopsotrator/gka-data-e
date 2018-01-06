cmake_minimum_required(VERSION 3.7)
project(gka-data-e)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(CMAKE_CXX_STANDARD 14)

find_package(Eina REQUIRED)
find_package(Ecore REQUIRED)
find_package(Evas REQUIRED)
find_package(Efl REQUIRED)
find_package(Elementary REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1" -DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/gka/data-e")

include(GNUInstallDirs)

add_subdirectory(handyfiles)
add_subdirectory(files)
add_subdirectory(po)
add_subdirectory(data_src)
add_subdirectory(data_lib)
add_subdirectory(data_lib_tests)
