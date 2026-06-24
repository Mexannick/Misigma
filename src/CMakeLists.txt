add_executable(Misigma WIN32
    main.cpp
)

target_link_libraries(Misigma PRIVATE
    WickedEngine_Windows
)

target_include_directories(Misigma PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/WickedEngine"
)

set_property(TARGET Misigma PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Misigma>")

set(WICKED_DXC "${CMAKE_SOURCE_DIR}/engine/WickedEngine/dxcompiler.dll")

add_custom_command(TARGET Misigma POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WICKED_DXC}"
        "$<TARGET_FILE_DIR:Misigma>/dxcompiler.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/engine/WickedEngine/shaders"
        "$<TARGET_FILE_DIR:Misigma>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/engine/Content"
        "$<TARGET_FILE_DIR:Misigma>/Content"
    COMMENT "Copying dxcompiler.dll, shaders and content..."
)
