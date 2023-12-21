cmake_minimum_required(VERSION 3.5.0)
project(g_engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(g_engine source/g_engine_2d.h source/audio_player.cpp source/draw_calls.cpp source/engine_modern.cpp source/engine_start.cpp source/FontRenderer.cpp 
source/function_loader.cpp source/g_primitive_funcs.h source/gl_defines.h source/image_draw.cpp source/image_loader.cpp source/input.cpp source/line.cpp source/line.cpp source/lodepng.cpp
source/lodepng.h source/matrix.cpp source/matrix.h source/shader.cpp source/window.cpp source/example.cpp source/audio.h source/backend.h source/vector.cpp 
source/vector.h source/primitive_renderer.cpp)

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