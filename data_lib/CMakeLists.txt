include_directories(
        ${EINA_INCLUDE_DIRS}
        ${EOLIAN_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
)

link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_library(data_lib sqlite_file.cpp csv_file.cpp csv_file.h)

target_include_directories (data_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(data_lib sqlite3 eina)
