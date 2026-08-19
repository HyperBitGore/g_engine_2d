#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/rendering/three_dee_renderer.hpp"
#include "gl_logger_output.hpp"
#include <cstdint>
#include <cstdio>

namespace {
struct pixel { uint8_t r, g, b, a; };

pixel sample(gore::drawpass& pass, GLint x, GLint y) {
    pixel value{};
    pass.bind();
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &value);
    pass.unbind();
    return value;
}
}

int main() {
    constexpr uint32_t size = 128;
    gore::g_engine_2d engine("Three-Dee Renderer Unit Test", size, size, 0, gore::LogType::NONE);
    auto renderer = gore::threedeerender::create(size, size);
    if (!renderer) return 1;
    gore::drawpass pass(size, size, GL_COLOR_ATTACHMENT0);

    engine.setRenderFunction([&] {
        pass.bind();
        glViewport(0, 0, size, size);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer->addTriangle({-1.0f, -1.0f, 0.0f},
                              {1.0f, -1.0f, 0.0f},
                              {0.0f, 1.0f, 0.0f});
        renderer->drawBuffer();
        pass.unbind();
    });

    if (!engine.updateWindow()) return 1;
    const pixel center = sample(pass, 64, 64);
    const bool passed = center.r > 200 && center.g > 200 &&
                        center.b > 200 && center.a == 255;
    const bool gl_logging_passed =
        check_gl_logger("glDrawElements", gore::draw_elements_log) &&
        check_gl_logger("glBindBuffer", gore::bind_buffer_log) &&
        check_gl_logger("glBufferData", gore::buffer_data_log) &&
        check_gl_logger("glBufferSubData", gore::buffer_sub_data_log) &&
        check_gl_logger("glBindVertexArray", gore::bind_vertex_array_log) &&
        check_gl_logger("glEnableVertexAttribArray", gore::enable_vertex_attrib_array_log) &&
        check_gl_logger("glBindBufferBase", gore::bind_buffer_base_log) &&
        check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    const bool call_data_passed =
        check_gl_logger_call("glBindBuffer array", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER)) &&
        check_gl_logger_call("glBindBuffer element", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_ELEMENT_ARRAY_BUFFER)) &&
        check_gl_logger_call("glBindBuffer shader storage", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER)) &&
        check_gl_logger_call("glBufferData array", gore::buffer_data_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER),
                             static_cast<GLsizeiptr>(sizeof(gore::threedee_vertex) * 3)) &&
        check_gl_logger_call("glBufferSubData array", gore::buffer_sub_data_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER), static_cast<GLintptr>(0),
                             static_cast<GLsizeiptr>(sizeof(gore::threedee_vertex) * 3)) &&
        check_gl_logger_call("glBindVertexArray unbind", gore::bind_vertex_array_log,
                             static_cast<GLuint>(0)) &&
        check_gl_logger_call("glEnableVertexAttribArray position",
                             gore::enable_vertex_attrib_array_log, static_cast<GLuint>(0)) &&
        check_gl_logger_call("glVertexAttribPointer position", gore::vertex_attrib_log,
                             static_cast<GLuint>(0), static_cast<GLint>(3),
                             static_cast<GLenum>(GL_FLOAT), static_cast<GLboolean>(GL_FALSE),
                             static_cast<GLsizei>(sizeof(gore::threedee_vertex)),
                             static_cast<const void*>(nullptr)) &&
        check_gl_logger_call("glVertexAttribIPointer model", gore::vertex_attrib_i_log,
                             static_cast<GLuint>(2), static_cast<GLint>(1),
                             static_cast<GLenum>(GL_INT)) &&
        check_gl_logger_call("glBindBufferBase matrices", gore::bind_buffer_base_log,
                             static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER),
                             static_cast<GLuint>(0));
    const bool draw_data_passed = check_gl_logger_call(
        "glDrawElements triangle", gore::draw_elements_log,
        static_cast<GLenum>(GL_TRIANGLES), static_cast<GLsizei>(3),
        static_cast<GLenum>(GL_UNSIGNED_INT), static_cast<const void*>(nullptr));
    print_gl_logger_output();
    std::printf("three-dee triangle color: %s\n", passed ? "PASS" : "FAIL");
    return passed && gl_logging_passed && call_data_passed && draw_data_passed ? 0 : 1;
}
