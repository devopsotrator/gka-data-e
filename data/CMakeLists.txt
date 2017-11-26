add_library(data sqlite_file.cpp)

target_include_directories (data PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(data sqlite3 eina)