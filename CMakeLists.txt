cmake_minimum_required (VERSION 2.8)
project (C_compiler)

SET(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
SET(BUILD_DIR "${ROOT_DIR}/build")
# SET(CMAKE_INSTALL_PREFIX ${BUILD_DIR} CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)
SET(CMAKE_INSTALL_PREFIX ${ROOT_DIR}/output CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/lib)
set(DEPEND_LIB_PATH ${BUILD_DIR}/lib)
set(DEPEND_INCLUDE_PATH ${DEP_ROOT_PATH}/include)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG_BUILD)
endif()

link_directories(${BUILD_DIR}/lib/)
include_directories(${BUILD_DIR}/0common/)
include_directories(${BUILD_DIR}/1lexer/)

add_subdirectory(${ROOT_DIR}/1lexer)
#add_subdirectory(${ROOT_DIR}/0common)
