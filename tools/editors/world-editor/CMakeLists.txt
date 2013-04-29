cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS} ${CROWN_BINARY_DIR})

include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(world-editor world-editor.cpp CrownDrawingArea.h CrownDrawingArea.cpp)

target_link_libraries(world-editor ${GTKMM_LIBRARIES} X11 GL crown)

install (TARGETS world-editor DESTINATION bin)
install (FILES ui/world-editor.glade DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ui)
