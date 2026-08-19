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
        check_gl_logger("glBindBufferBase", gore::bind_buffer_base_log) &&
        check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    const bool draw_data_passed = check_gl_logger_call(
        "glDrawElements triangle", gore::draw_elements_log,
        static_cast<GLenum>(GL_TRIANGLES), static_cast<GLsizei>(3),
        static_cast<GLenum>(GL_UNSIGNED_INT), static_cast<const void*>(nullptr));
    print_gl_logger_output();
    std::printf("three-dee triangle color: %s\n", passed ? "PASS" : "FAIL");
    return passed && gl_logging_passed && draw_data_passed ? 0 : 1;
}
