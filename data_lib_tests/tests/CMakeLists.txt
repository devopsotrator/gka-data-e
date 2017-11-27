include_directories(
        ${EINA_INCLUDE_DIRS}
        ${EOLIAN_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_executable(runBasicTests sqlite_file_test.cpp)

target_link_libraries(runBasicTests gtest gtest_main)
target_link_libraries(runBasicTests data_lib elementary ecore evas eina)
