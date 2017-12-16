include_directories(
        ${EINA_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
)

link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_executable(data_lib_tests sqlite_file_test.cpp test_main.cpp csv_file_test.cpp)

target_link_libraries(data_lib_tests data_lib elementary ecore evas eina)
