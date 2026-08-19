#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/rendering/wireframe_renderer.hpp"
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
    constexpr uint32_t size = 64;
    gore::g_engine_2d engine("Wireframe Renderer Unit Test", size, size, 0, gore::LogType::NONE);
    auto renderer = gore::wireframe_renderer::create(size, size);
    gore::drawpass pass(size, size, GL_COLOR_ATTACHMENT0);

    engine.setRenderFunction([&] {
        pass.bind();
        glViewport(0, 0, size, size);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer->setColor({0, 1, 0, 1});
        renderer->addLine({-0.5f, 0.0f, 1.0f}, {0.5f, 0.0f, 1.0f});
        renderer->drawBuffer();
        pass.unbind();
    });

    if (!engine.updateWindow()) return 1;
    bool passed = false;
    for (GLint y = 29; y <= 34 && !passed; ++y) {
        for (GLint x = 28; x <= 36; ++x) {
            const pixel value = sample(pass, x, y);
            if (value.g > 200 && value.r < 10 && value.b < 10 && value.a > 0) {
                passed = true;
                break;
            }
        }
    }
    const bool gl_logging_passed =
        check_gl_logger("glDrawArrays", gore::draw_arrays_log) &&
        check_gl_logger("glBindBuffer", gore::bind_buffer_log) &&
        check_gl_logger("glBufferData", gore::buffer_data_log) &&
        check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    const bool draw_data_passed = check_gl_logger_call(
        "glDrawArrays wireframe", gore::draw_arrays_log,
        static_cast<GLenum>(GL_LINES), static_cast<GLint>(0), static_cast<GLsizei>(2));
    print_gl_logger_output();
    std::printf("wireframe line color: %s\n", passed ? "PASS" : "FAIL");
    return passed && gl_logging_passed && draw_data_passed ? 0 : 1;
}
