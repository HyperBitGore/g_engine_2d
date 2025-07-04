cmake_minimum_required(VERSION 4.0.0)
if(NOT CMAKE_BUILD_TYPE)
message("build type not set, setting to debug")
set(CMAKE_BUILD_TYPE Debug)
endif()

project(g_engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(g_engine source/g_engine_2d.hpp source/audio/audio_player.cpp source/engine_modern.cpp source/engine_start.cpp source/rendering/font_renderer.cpp 
source/function_loader.cpp source/gl_defines.hpp source/util/hashmap.hpp source/img_loading/image_loader.cpp source/input.cpp source/img_loading/lodepng.cpp
source/img_loading/lodepng.h source/util/matrix.cpp source/util/matrix.hpp source/util/shader.cpp source/window.cpp example/example.cpp source/audio/audio.hpp source/backend.hpp source/util/vector.cpp 
source/util/vector.hpp source/rendering/primitive_renderer.cpp source/rendering/image_renderer.cpp source/img_loading/image_atlas.cpp source/img_loading/bmp_loader.cpp source/util/shader.hpp source/img_loading/image_loader.hpp source/rendering/image_renderer.hpp
source/rendering/primitive_renderer.hpp source/rendering/font_renderer.hpp)

#target_compile_options(g_engine PRIVATE /utf-8)
find_package(OpenGL REQUIRED)

if(WIN32)
#SET(LINK_LIBRARY gdi32 user32 opengl32 ole32)
target_link_libraries(g_engine gdi32 user32 opengl32 ole32)
target_compile_options(g_engine PRIVATE /utf-8)
endif(WIN32)
if(UNIX)
find_package(X11 REQUIRED)
find_package(ALSA REQUIRED)
message(STATUS "X11_FOUND: ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES: ${X11_LIBRARIES}")
message(STATUS "X11_Xkblib_FOUND: ${X11_Xkb_FOUND}")
message(STATUS "X11_Xkblib_LIB: ${X11_Xkb_INCLUDE_PATH}")
if (NOT X11_Xkb_FOUND)
    message(FATAL_ERROR "XKB extension not found")
endif()
target_include_directories(g_engine PRIVATE ${X11_INCLUDE_DIR} ${X11_Xkb_INCLUDE_PATH})
target_link_libraries(g_engine OpenGL::GL dl X11::X11 X11::Xkb ALSA::ALSA)
endif(UNIX)

target_include_directories(g_engine PUBLIC include/GL)