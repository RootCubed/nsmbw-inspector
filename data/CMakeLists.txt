add_custom_target(copyAssets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/ ${CMAKE_BINARY_DIR}/bin/Debug
)