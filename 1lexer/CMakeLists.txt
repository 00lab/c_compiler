
aux_source_directory(. SRC_LIST)

# add_library(lexer_lib SHARED ${SRC_LIST})

# target_include_directories(
#     lexer_lib
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/../0common)

# target_link_libraries(lexer_lib PUBLIC -Wl,-rpath-link=${DEPEND_LIB_PATH})

# # # make intall行为
# install(TARGETS lexer_lib RUNTIME DESTINATION lib)



# add_executable(lexer_bin ${CMAKE_CURRENT_SOURCE_DIR}/main_lexer.cpp)
add_executable(lexer_bin ${SRC_LIST})

target_include_directories(
    lexer_bin
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../0common)

target_link_libraries(lexer_bin PUBLIC -Wl,-rpath-link=${DEPEND_LIB_PATH})

install(TARGETS lexer_bin RUNTIME DESTINATION bin)

