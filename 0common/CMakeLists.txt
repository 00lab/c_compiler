
aux_source_directory(. SRC_LIST)

add_library(common_lib SHARED ${SRC_LIST})

target_include_directories(
    common_lib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(common_lib PUBLIC -Wl,-rpath-link=${DEPEND_LIB_PATH})

# # make intall行为
install(TARGETS common_lib RUNTIME DESTINATION lib)
