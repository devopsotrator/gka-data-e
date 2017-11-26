cmake_minimum_required(VERSION 3.7)
project(gka_data_e)

set(CMAKE_CXX_STANDARD 17)

find_package(Eina REQUIRED)
find_package(Eo REQUIRED)
find_package(Eolian REQUIRED)
find_package(Ecore REQUIRED)
find_package(Evas REQUIRED)
find_package(Efl REQUIRED)
find_package(Elementary REQUIRED)

include_directories(
        ${EINA_INCLUDE_DIRS}
        ${EOLIAN_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
)

link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_executable(gka_data_e main.cpp data_ui.cpp data_menu.cpp data_menu.h data_label_preferences.cpp data_label_preferences.h)

add_subdirectory(data)
add_subdirectory(data_tests)

target_link_libraries(gka_data_e elementary ecore evas eina)
target_link_libraries(gka_data_e data)
