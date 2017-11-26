#project(runBasicTests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_executable(runBasicTests
        sqlite_file_test.cpp)

target_link_libraries(runBasicTests gtest gtest_main)
target_link_libraries(runBasicTests data elementary ecore evas eina)
