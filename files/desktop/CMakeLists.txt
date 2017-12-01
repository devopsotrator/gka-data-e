configure_file(data.desktop.in data.desktop)

install(PROGRAMS ${CMAKE_BINARY_DIR}/files/desktop/data.desktop DESTINATION share/applications)

install(FILES gka_data_e.svg DESTINATION share/icons/hicolor/scalable/apps)
