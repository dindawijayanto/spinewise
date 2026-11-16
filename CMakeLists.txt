cmake_minimum_required(VERSION 3.16)
project(Spinewise VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Runtime statis supaya .exe jalan di laptop lain tanpa install apa pun
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ---------------------------------------------------------------------------
# Library pihak ketiga (di-vendor di external/, build tanpa internet)
# ---------------------------------------------------------------------------

# GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD (OpenGL 3.3 Core, dibuat dengan glad2)
add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE external/glm)

# tinyobjloader (header-only)
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE external/tinyobjloader)

# Dear ImGui + backend GLFW/OpenGL3
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

# ---------------------------------------------------------------------------
# Aplikasi
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SPINEWISE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)

add_executable(Spinewise ${SPINEWISE_SOURCES})
target_include_directories(Spinewise PRIVATE src)
target_link_libraries(Spinewise PRIVATE glfw glad glm imgui tinyobjloader)
target_compile_definitions(Spinewise PRIVATE GLFW_INCLUDE_NONE)

if(MSVC)
    target_compile_options(Spinewise PRIVATE /W4 /utf-8)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Spinewise)
else()
    target_compile_options(Spinewise PRIVATE -Wall -Wextra)
endif()

if(MINGW)
    target_link_options(Spinewise PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Copy shaders/ dan assets/ ke samping .exe setiap build
add_custom_command(TARGET Spinewise POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Spinewise>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Spinewise>/assets
    COMMENT "Copy shaders/ dan assets/ ke folder output"
)
