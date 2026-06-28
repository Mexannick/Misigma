add_executable(Misigma WIN32
    main.cpp

    # Core
    core/GameState.h
    core/Application.h
    core/Application.cpp

    # States
    states/LoadingState.h
    states/LoadingState.cpp
    states/MainMenuState.h
    states/MainMenuState.cpp
    states/GameplayState.h
    states/GameplayState.cpp
    states/OptionsState.h
    states/OptionsState.cpp
    states/LobbyState.h
    states/LobbyState.cpp

    # Network
    net/Packets.h
    net/NetManager.h
    net/NetManager.cpp
)

target_link_libraries(Misigma PRIVATE
    WickedEngine_Windows
)

target_include_directories(Misigma PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/WickedEngine"
    # Allow #include "core/...", "states/...", "net/..." from anywhere in src/
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_property(TARGET Misigma PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Misigma>")

# ── Post-build: copy engine runtime files ─────────────────────────────────────
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
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/game_shaders"
        "$<TARGET_FILE_DIR:Misigma>/shaders"
    COMMENT "Copying dxcompiler.dll, shaders and content..."
)
