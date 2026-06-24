add_executable(Misigma WIN32
    main.cpp
)

target_link_libraries(Misigma PRIVATE
    WickedEngine_Windows
)

target_include_directories(Misigma PRIVATE
    "${CMAKE_SOURCE_DIR}/engine/WickedEngine"
)

add_custom_command(TARGET Misigma POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/engine/WickedEngine/shaders"
        "$<TARGET_FILE_DIR:Misigma>/shaders"
)
