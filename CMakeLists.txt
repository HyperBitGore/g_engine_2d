cmake_minimum_required(VERSION 4.0.0)
if(NOT CMAKE_BUILD_TYPE)
message("build type not set, setting to debug")
set(CMAKE_BUILD_TYPE Debug)
endif()

project(g_engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UNIT_TEST "Enable unit-test-only code" OFF)
if(UNIT_TEST)
    add_compile_definitions(_UNIT_TEST_)
endif()

enable_testing()

set(ENGINE_SOURCES
    include/g_engine/g_engine_2d.hpp
    include/g_engine/audio/audio_player.cpp
    include/g_engine/engine_modern.cpp
    include/g_engine/engine_start.cpp
    include/g_engine/rendering/font_renderer.cpp
    include/g_engine/function_loader.cpp
    include/g_engine/gl_defines.hpp
    include/g_engine/util/hashmap.hpp
    include/g_engine/img_loading/image_loader.cpp
    include/g_engine/input.cpp
    include/g_engine/util/matrix.cpp
    include/g_engine/util/matrix.hpp
    include/g_engine/util/shader.cpp
    include/g_engine/window.cpp
    include/g_engine/audio/audio.hpp
    include/g_engine/backend.hpp
    include/g_engine/util/vector.cpp
    include/g_engine/util/vector.hpp
    include/g_engine/rendering/primitive_renderer.cpp
    include/g_engine/rendering/primitive_renderer_shader.hpp
    include/g_engine/rendering/image_renderer.cpp
    include/g_engine/img_loading/image_atlas.cpp
    include/g_engine/img_loading/bmp_loader.cpp
    include/g_engine/util/shader.hpp
    include/g_engine/img_loading/image_loader.hpp
    include/g_engine/rendering/image_renderer.hpp
    include/g_engine/rendering/image_renderer_shader.hpp
    include/g_engine/rendering/primitive_renderer.hpp
    include/g_engine/rendering/font_renderer.hpp
    include/g_engine/file_loading/font_loader.cpp
    include/g_engine/file_loading/font_loader.hpp
    include/g_engine/img_loading/png_loader.cpp
    include/g_engine/img_loading/common.hpp
    include/g_engine/img_loading/inflate.hpp
    include/g_engine/file_loading/file_reader.hpp
    include/g_engine/file_loading/file_reader.cpp
    include/g_engine/util/logging.hpp
    include/g_engine/rendering/renderer.hpp
    include/g_engine/rendering/wireframe_renderer.hpp
    include/g_engine/rendering/wireframe_renderer.cpp
    include/g_engine/rendering/three_dee_renderer.hpp
    include/g_engine/rendering/three_dee_renderer.cpp
    include/g_engine/file_loading/model_loading/obj_loader.cpp
    include/g_engine/file_loading/model_loading/gltf_loader.cpp
    include/g_engine/file_loading/model_loading/model.cpp
    include/g_engine/rendering/camera.hpp
    include/g_engine/file_loading/json.cpp
)

add_executable(g_engine ${ENGINE_SOURCES} tests/example.cpp)

# Tests
add_executable(test_audio
    include/g_engine/audio/audio_player.cpp
    include/g_engine/audio/audio.hpp
    include/g_engine/util/logging.hpp
    tests/audio.cpp
)
add_executable(test_images      ${ENGINE_SOURCES} tests/images.cpp)
add_executable(test_primitives  ${ENGINE_SOURCES} tests/primitives.cpp)
add_executable(test_text        ${ENGINE_SOURCES} tests/text.cpp)
add_executable(test_frame_limit ${ENGINE_SOURCES} tests/frame_limit.cpp)
add_executable(test_image_stress ${ENGINE_SOURCES} tests/image_render_stress.cpp)
add_executable(test_matrix_vector
    include/g_engine/util/matrix.cpp
    include/g_engine/util/matrix.hpp
    include/g_engine/util/vector.cpp
    include/g_engine/util/vector.hpp
    tests/unit_tests/matrix_vector_test.cpp
)
add_executable(test_primitive_unit ${ENGINE_SOURCES} tests/unit_tests/primitive_unit_test.cpp)
add_executable(test_image_unit ${ENGINE_SOURCES} tests/unit_tests/image_unit_test.cpp)
add_executable(test_wireframe_unit ${ENGINE_SOURCES} tests/unit_tests/wireframe_unit_test.cpp)
add_executable(test_font_unit ${ENGINE_SOURCES} tests/unit_tests/font_unit_test.cpp)
add_executable(test_three_dee_unit ${ENGINE_SOURCES} tests/unit_tests/three_dee_unit_test.cpp)
add_executable(test_crawl ${ENGINE_SOURCES} tests/crawl_test.cpp)

add_executable(test_rotate_cube_soft ${ENGINE_SOURCES} tests/rotating_cube_software.cpp)

add_executable(test_texture_unit_overflow ${ENGINE_SOURCES} tests/texture_unit_overflow.cpp)

add_executable(test_three_dee_texture_split ${ENGINE_SOURCES} tests/three_dee_texture_split.cpp)

add_executable(test_instance_render ${ENGINE_SOURCES} tests/instance_render.cpp)

add_executable(test_instance_render_mutation ${ENGINE_SOURCES} tests/instance_render_mutation.cpp)

add_executable(test_maintain_renderer_view ${ENGINE_SOURCES} tests/maintain_renderer_view.cpp)

add_executable(test_dual_engine ${ENGINE_SOURCES} tests/dual_engine.cpp)

add_test(NAME matrix_vector COMMAND test_matrix_vector)
add_test(NAME primitive_unit COMMAND test_primitive_unit)
add_test(NAME image_unit COMMAND test_image_unit)
add_test(NAME wireframe_unit COMMAND test_wireframe_unit)
add_test(NAME font_unit COMMAND test_font_unit)
add_test(NAME three_dee_unit COMMAND test_three_dee_unit)

#target_compile_options(g_engine PRIVATE /utf-8)
find_package(OpenGL REQUIRED)

set(VISUAL_TARGETS g_engine test_images test_primitives test_primitive_unit test_image_unit test_wireframe_unit test_font_unit test_three_dee_unit test_text test_frame_limit test_image_stress test_rotate_cube_soft test_crawl test_texture_unit_overflow test_three_dee_texture_split test_instance_render test_instance_render_mutation test_maintain_renderer_view test_dual_engine)

if(WIN32)
foreach(tgt ${VISUAL_TARGETS})
    target_link_libraries(${tgt} gdi32 user32 opengl32 ole32)
    target_compile_options(${tgt} PRIVATE /utf-8)
endforeach()
target_link_libraries(test_audio ole32)
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
foreach(tgt ${VISUAL_TARGETS})
    target_include_directories(${tgt} PRIVATE ${X11_INCLUDE_DIR} ${X11_Xkb_INCLUDE_PATH})
    target_link_libraries(${tgt} OpenGL::GL dl X11::X11 X11::Xkb ALSA::ALSA)
endforeach()
target_link_libraries(test_audio ALSA::ALSA)
endif(UNIX)

foreach(tgt ${VISUAL_TARGETS})
    target_include_directories(${tgt} PUBLIC include/GL)
endforeach()