cmake_minimum_required(VERSION 3.6)
project(raidix)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} )


find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)

set( CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR} )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories( ${GLIB2_INCLUDE_DIRS} )
link_directories( ${GLIB2_LIBRARY_DIRS} )
add_definitions( ${GLIB2_CFLAGS_OTHER} )
set(SOURCE_FILES server_main.c storage.c macros.h minini/minIni.c)
add_executable(server ${SOURCE_FILES} )
target_link_libraries( server ${GLIB2_LIBRARIES} pthread )

add_executable( client client_main.c )

#set(SOURCE_FILES client_main.c storage.c storage.h)
#add_executable( client ${SOURCE_FILES} )