cmake_minimum_required(VERSION 3.15)
project(GraphicsEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use DirectXTK12 from NuGet package
set(DIRECTXTK12_DIR "${CMAKE_SOURCE_DIR}/directxtk12_desktop_2019.2025.7.10.1")
set(DIRECTXTK12_INCLUDE_DIR "${DIRECTXTK12_DIR}/include")
set(DIRECTXTK12_LIB_DIR "${DIRECTXTK12_DIR}/native/lib/x64/Release")

add_library(DirectXTK12 STATIC IMPORTED)
set_target_properties(DirectXTK12 PROPERTIES
    IMPORTED_LOCATION "${DIRECTXTK12_LIB_DIR}/DirectXTK12.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${DIRECTXTK12_INCLUDE_DIR}"
)

# Source files organized by directory
set(CORE_SOURCES
    src/core/main.cpp
    src/core/Engine.cpp
)

set(CORE_HEADERS
    include/core/Engine.h
    include/core/IRenderer.h
    include/core/Logger.h
)

set(RENDERER_SOURCES
    src/renderers/GDIRenderer.cpp
    src/renderers/DX12Renderer.cpp
)

set(RENDERER_HEADERS
    include/renderers/GDIRenderer.h
    include/renderers/DX12Renderer.h
)

# Main executable
add_executable(GraphicsEngine
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${RENDERER_SOURCES}
    ${RENDERER_HEADERS}
)

# Include directories
target_include_directories(GraphicsEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/renderers
    ${CMAKE_SOURCE_DIR}/include/third_party
)

# Set Windows subsystem
set_target_properties(GraphicsEngine PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Link libraries
target_link_libraries(GraphicsEngine
    DirectXTK12
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    gdi32.lib
    user32.lib
    kernel32.lib
    msimg32.lib
)

# Set Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GraphicsEngine)

# Add compile definitions for Windows
target_compile_definitions(GraphicsEngine PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# Copy sprite font assets to output directory
add_custom_command(TARGET GraphicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/arial24.spritefont"
        "$<TARGET_FILE_DIR:GraphicsEngine>/arial24.spritefont"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/arial120.spritefont"
        "$<TARGET_FILE_DIR:GraphicsEngine>/arial120.spritefont"
    COMMENT "Copying sprite font assets to output directory"
)

# Organize files in Visual Studio Solution Explorer
source_group("Core\\Source" FILES ${CORE_SOURCES})
source_group("Core\\Headers" FILES ${CORE_HEADERS})
source_group("Renderers\\Source" FILES ${RENDERER_SOURCES})
source_group("Renderers\\Headers" FILES ${RENDERER_HEADERS})
