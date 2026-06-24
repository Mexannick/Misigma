cmake_minimum_required(VERSION 3.19)

project(Misigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WICKED_EDITOR OFF CACHE BOOL "" FORCE)
set(WICKED_TESTS OFF CACHE BOOL "" FORCE)
set(WICKED_IMGUI_EXAMPLE OFF CACHE BOOL "" FORCE)
set(WICKED_WINDOWS_TEMPLATE OFF CACHE BOOL "" FORCE)
set(WICKED_LINUX_TEMPLATE OFF CACHE BOOL "" FORCE)
set(WICKED_EMBED_SHADERS OFF CACHE BOOL "" FORCE)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/engine/CMakeLists.txt")
    message(FATAL_ERROR "WickedEngine submodule not found.\nRun: git submodule update --init --recursive")
endif()

add_subdirectory(engine)

target_include_directories(WickedEngine_common SYSTEM PUBLIC
    "${CMAKE_SOURCE_DIR}/engine/WickedEngine/Utility/dx12"
)

add_subdirectory(src)

