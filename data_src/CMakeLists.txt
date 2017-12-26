configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${EINA_INCLUDE_DIRS}
        ${ECORE_INCLUDE_DIRS}
        ${EOLIAN_INCLUDE_DIRS}
        ${EVAS_INCLUDE_DIRS}
        ${ELEMENTARY_INCLUDE_DIRS}
)

link_directories (${PC_LIBELEMENTARY_LIBRARY_DIRS})

add_executable(${PROJECT_NAME_LOWER} main.cpp data_ui.cpp data_menu.cpp data_menu.h data_label_preferences.cpp data_label_preferences.h data_edit_record.cpp data_edit_record.h data_table_preferences.cpp data_table_preferences.h)

target_link_libraries(${PROJECT_NAME_LOWER} elementary ecore evas eina)
target_link_libraries(${PROJECT_NAME_LOWER} data_lib)

install(TARGETS ${PROJECT_NAME_LOWER} RUNTIME DESTINATION bin)
