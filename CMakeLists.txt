cmake_minimum_required (VERSION 3.1)
project (C_compiler)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
set(BUILD_DIR "${ROOT_DIR}/build")
set(OUTPUT_DIR "${ROOT_DIR}/output")
# SET(CMAKE_INSTALL_PREFIX ${BUILD_DIR} CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)
set(CMAKE_INSTALL_PREFIX ${OUTPUT_DIR} CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/lib) # 默认放静态库的位置
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}/lib) # 默认放动态库的位置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${BUILD_DIR}/bin) # 默认可执行文件的位置
set(DEPEND_LIB_PATH ${OUTPUT_DIR}/lib)
set(DEPEND_INCLUDE_PATH ${DEP_ROOT_PATH}/include)

set(CMAKE_CXX_FLAGS "-std=c++11")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG_BUILD)
endif()

link_directories(${OUTPUT_DIR}/lib/)
include_directories(${BUILD_DIR}/0common/)
include_directories(${BUILD_DIR}/1lexer/)

add_subdirectory(${ROOT_DIR}/0common)
add_subdirectory(${ROOT_DIR}/1lexer)
