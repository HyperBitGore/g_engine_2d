cmake_minimum_required(VERSION 3.5.0)
if(NOT CMAKE_BUILD_TYPE)
message("build type not set, setting to debug")
set(CMAKE_BUILD_TYPE Debug)
endif()

project(g_engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(g_engine source/g_engine_2d.hpp source/audio_player.cpp source/engine_modern.cpp source/engine_start.cpp source/font_renderer.cpp 
source/function_loader.cpp source/g_primitive_funcs.h source/gl_defines.hpp source/image_loader.cpp source/input.cpp source/lodepng.cpp
source/lodepng.h source/matrix.cpp source/matrix.hpp source/shader.cpp source/window.cpp source/example.cpp source/audio.hpp source/backend.hpp source/vector.cpp 
source/vector.hpp source/primitive_renderer.cpp source/image_renderer.cpp source/image_atlas.cpp source/img_loading/bmp_loader.cpp source/shader.hpp source/image_loader.hpp source/image_renderer.hpp
source/primitive_renderer.hpp source/font_renderer.hpp)

target_compile_options(g_engine PRIVATE /utf-8)

if(WIN32)
SET(LINK_LIBRARY gdi32 user32 opengl32 ole32)
add_compile_options(/utf-8)
endif(WIN32)
if(UNIX)
SET(LINK_LIBRARY)
endif(UNIX)

target_include_directories(g_engine PUBLIC include/GL)
target_link_libraries(g_engine ${LINK_LIBRARY})