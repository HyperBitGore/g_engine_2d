#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/img_loading/image_loader.hpp"
#include "gl_logger_output.hpp"
#include <cstdint>
#include <cstdio>
#if defined(_UNIT_TEST_)
namespace {
constexpr uint32_t SIZE = 64;

struct pixel {
    uint8_t r, g, b, a;
};

pixel sample(gore::drawpass& pass, GLint x, GLint y) {
    pixel value{};
    pass.bind();
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &value);
    pass.unbind();
    return value;
}

bool matches(pixel actual, pixel expected) {
    return actual.r == expected.r && actual.g == expected.g &&
           actual.b == expected.b && actual.a == expected.a;
}
}

int main() {
    gore::g_engine_2d engine("Image Renderer Unit Test", SIZE, SIZE, 0, gore::LogType::NONE);
    auto image_renderer = gore::createImageRenderer(SIZE, SIZE);
    auto grayscale_renderer =
        gore::image_renderer::create<gore::grayscalerenderer>(SIZE, SIZE);
    auto image = gore::imageloader::createBlank(4, 4, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            gore::imageloader::setPixel(image, x, y, 200, 30, 60, 255);
        }
    }
    gore::imageloader::createTexture(image, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    gore::imageloader::updateIMG(image);
    gore::drawpass pass(SIZE, SIZE, GL_COLOR_ATTACHMENT0);

    engine.setRenderFunction([&] {
        pass.bind();
        glViewport(0, 0, SIZE, SIZE);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        image_renderer->drawImage(image, {4, 4}, {20, 20});
        grayscale_renderer->drawImage(image, {36, 4}, {20, 20});
        pass.unbind();
    });

    if (!engine.updateWindow()) return 1;

    const pixel source = sample(pass, 10, 54);
    const pixel gray = sample(pass, 42, 54);
    const bool passed = matches(source, {200, 30, 60, 255}) &&
                        matches(gray, {200, 200, 200, 255});
    const bool gl_logging_passed =
        check_gl_logger("glDrawArrays", gore::draw_arrays_log) &&
        check_gl_logger("glBindBuffer", gore::bind_buffer_log) &&
        check_gl_logger("glBufferData", gore::buffer_data_log) &&
        check_gl_logger("glBindVertexArray", gore::bind_vertex_array_log) &&
        check_gl_logger("glEnableVertexAttribArray", gore::enable_vertex_attrib_array_log) &&
        check_gl_logger("glVertexAttribIPointer", gore::vertex_attrib_i_log) &&
        check_gl_logger("glBindBufferBase", gore::bind_buffer_base_log) &&
        check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    const bool call_data_passed =
        check_gl_logger_call("glBindBuffer array", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER)) &&
        check_gl_logger_call("glBindBuffer shader storage", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER)) &&
        check_gl_logger_call("glBufferData image", gore::buffer_data_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER),
                             static_cast<GLsizeiptr>(sizeof(gore::image_render_vertex) * 6)) &&
        check_gl_logger_call("glBindVertexArray unbind", gore::bind_vertex_array_log,
                             static_cast<GLuint>(0)) &&
        check_gl_logger_call("glEnableVertexAttribArray position",
                             gore::enable_vertex_attrib_array_log, static_cast<GLuint>(0)) &&
        check_gl_logger_call("glVertexAttribPointer position", gore::vertex_attrib_log,
                             static_cast<GLuint>(0), static_cast<GLint>(2),
                             static_cast<GLenum>(GL_FLOAT), static_cast<GLboolean>(GL_FALSE),
                             static_cast<GLsizei>(sizeof(gore::image_render_vertex)),
                             static_cast<const void*>(nullptr)) &&
        check_gl_logger_call("glVertexAttribIPointer texture", gore::vertex_attrib_i_log,
                             static_cast<GLuint>(4), static_cast<GLint>(1),
                             static_cast<GLenum>(GL_UNSIGNED_INT)) &&
        check_gl_logger_call("glBindBufferBase texture", gore::bind_buffer_base_log,
                             static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER),
                             static_cast<GLuint>(0));
    const bool draw_data_passed = check_gl_logger_call(
        "glDrawArrays image", gore::draw_arrays_log,
        static_cast<GLenum>(GL_TRIANGLES), static_cast<GLint>(0), static_cast<GLsizei>(6));
    print_gl_logger_output();
    std::printf("image color: %s\n", matches(source, {200, 30, 60, 255}) ? "PASS" : "FAIL");
    std::printf("grayscale color: %s\n", matches(gray, {200, 200, 200, 255}) ? "PASS" : "FAIL");
    return passed && gl_logging_passed && call_data_passed && draw_data_passed ? 0 : 1;
}
#endif