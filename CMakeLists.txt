cmake_minimum_required(VERSION 3.0)
project(filebd)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -std=c11 )


find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)

set( CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR} )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories( ${GLIB2_INCLUDE_DIRS} )
link_directories( ${GLIB2_LIBRARY_DIRS} )
add_definitions( ${GLIB2_CFLAGS_OTHER} -D_POSIX_C_SOURCE=199309L )
set(SOURCE_FILES server_main.c storage.c minini/minIni.c macros.c)
add_executable(server ${SOURCE_FILES} )
target_link_libraries( server ${GLIB2_LIBRARIES} pthread )

add_executable( client client_main.c )
target_link_libraries( client pthread )
