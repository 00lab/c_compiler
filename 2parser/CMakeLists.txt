
aux_source_directory(. SRC_LIST)

add_library(parser_lib SHARED ${SRC_LIST})

target_include_directories(
    parser_lib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../0common
    ${CMAKE_CURRENT_SOURCE_DIR}/../1lexer)

target_link_libraries(parser_lib PUBLIC -Wl,-rpath-link=${DEPEND_LIB_PATH} common_lib lexer_lib)

# # make intall行为
#install(TARGETS lexer_lib RUNTIME DESTINATION lib)
install (TARGETS parser_lib DESTINATION lib)


add_executable(parser_bin ${CMAKE_CURRENT_SOURCE_DIR}/main_paser.cpp)
# add_executable(lexer_bin ${SRC_LIST})

target_include_directories(
    parser_bin
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../0common
    ${CMAKE_CURRENT_SOURCE_DIR}/../1lexer)
# set(CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
target_link_libraries(parser_bin PUBLIC -Wl,-rpath-link=${DEPEND_LIB_PATH} -Wl,-Bdynamic common_lib lexer_lib parser_lib)

install(TARGETS parser_bin RUNTIME DESTINATION bin)
