add_custom_target(copyAssets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/iosevka-ss01-regular.ttf ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>/iosevka-ss01-regular.ttf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/sarasa-term-j-regular.ttf ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>/sarasa-term-j-regular.ttf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/structures.txt ${CMAKE_BINARY_DIR}/bin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>/structures.txt
)