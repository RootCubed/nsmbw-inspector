add_custom_target(copyAssets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/iosevka-ss01-regular.ttf ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/structures.txt ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>
)