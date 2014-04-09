cmake_minimum_required(VERSION 2.8)

project(crown-tests)

file(GLOB_RECURSE CROWN_TESTS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE CROWN_TESTS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set (CROWN_TESTS_INCLUDE_DIRS "")
foreach (_headerFile ${CROWN_TESTS_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND CROWN_TESTS_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES CROWN_TESTS_INCLUDE_DIRS)

include_directories(${CROWN_TESTS_INCLUDE_DIRS})

add_executable(crown-tests ${CROWN_TESTS_SOURCES})

#target_link_libraries(crown-tests crown)

install (TARGETS crown-tests DESTINATION tests)
