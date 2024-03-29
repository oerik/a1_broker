CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
get_filename_component(BASENAME ${CMAKE_SOURCE_DIR} NAME)

PROJECT(${BASENAME} C)
set(PROJECT_NAME a1_broker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS ON)

exec_program(
    "git"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "describe"
    OUTPUT_VARIABLE VERSION )

add_definitions(-D_GNU_SOURCE -Wall -Wextra -Werror -ggdb -DGITREVISION=${VERSION} -O2)

find_package (Threads)

aux_source_directory(. SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})
SET(CMAKE_INSTALL_PREFIX /usr)
include_directories("${PROJECT_BINARY_DIR}")

target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} dl ) 

# Add "tags" target and make my_project depending on this target.
set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_target(tags
    COMMAND ctags -R -e --fields=+iaS --extra=+q .
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_dependencies(${PROJECT_NAME} tags)


# Install executable
install (TARGETS ${PROJECT_NAME} DESTINATION bin)

add_subdirectory( systemd )

# Stuff for automatically packaging for debian
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Erik Oomen")
set(CPACK_PACKAGE_VERSION "1.0.0")
SET(CPACK_PACKAGE_CONTACT "erik@nib4.nl")

INCLUDE(CPack)
