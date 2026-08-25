#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/file_loading/font_loader.hpp"
#include "../../include/g_engine/rendering/font_renderer.hpp"
#include "gl_logger_output.hpp"
#include <cstdint>
#include <cstdio>
#if defined(_UNIT_TEST_)
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
    gore::g_engine_2d engine("Font Renderer Unit Test", size, size, 0, gore::LogType::NONE);
    gore::font font = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 0, 735);
    gore::fontraster::rasterizeFont(&font, 32, engine.getDPI(), 0xFFFFFFFF, 32, 127);
    auto renderer = gore::fontrenderer::create(size, size);
    auto image_renderer = gore::createImageRenderer(size, size);
    gore::drawpass pass(size, size, GL_COLOR_ATTACHMENT0);
    bool batching_passed = false;

    engine.setRenderFunction([&] {
        pass.bind();
        glViewport(0, 0, size, size);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer->setColor({1, 0, 0, 1});
        const size_t outline_draws_before_add = gore::draw_arrays_log.calls.size();
        renderer->addText("A", &font, 16, 32, 32, engine.getDPI());
        const bool add_text_batched =
            gore::draw_arrays_log.calls.size() == outline_draws_before_add;
        renderer->drawBuffer();
        const bool outline_draw_buffer_called =
            gore::draw_arrays_log.hasCallWithPrefix(static_cast<GLenum>(GL_LINES),
                                                    static_cast<GLint>(0));

        engine.enable(GL_BLEND);
        const size_t raster_draws_before_add = gore::draw_arrays_log.calls.size();
        gore::fontraster::addRasterText(&font, image_renderer.get(), "A",
                                        64, 64, 32, engine.getDPI());
        const bool add_raster_text_batched =
            gore::draw_arrays_log.calls.size() == raster_draws_before_add;
        image_renderer->drawBuffer();
        engine.disable(GL_BLEND);
        pass.unbind();

        batching_passed = add_text_batched && outline_draw_buffer_called &&
                          add_raster_text_batched;
    });

    if (!engine.updateWindow()) return 1;

    bool outline_found = false;
    bool raster_found = false;
    for (GLint y = 0; y < static_cast<GLint>(size) &&
                       (!outline_found || !raster_found); ++y) {
        for (GLint x = 0; x < static_cast<GLint>(size); ++x) {
            const pixel value = sample(pass, x, y);
            if (value.r > 200 && value.g < 10 && value.b < 10 && value.a > 0) {
                outline_found = true;
            }
            if (value.r > 200 && value.g > 200 && value.b > 200 && value.a > 0 &&
                x >= 64) {
                raster_found = true;
            }
        }
    }
    const bool gl_logging_passed =
        check_gl_logger("glDrawArrays", gore::draw_arrays_log) &&
        check_gl_logger("glBindBuffer", gore::bind_buffer_log) &&
        check_gl_logger("glBufferData", gore::buffer_data_log) &&
        check_gl_logger("glBindVertexArray", gore::bind_vertex_array_log) &&
        check_gl_logger("glEnableVertexAttribArray", gore::enable_vertex_attrib_array_log) &&
        check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    const bool call_data_passed =
        check_gl_logger_call("glBindBuffer array", gore::bind_buffer_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER)) &&
        check_gl_logger_call("glBufferData font", gore::buffer_data_log,
                             static_cast<GLenum>(GL_ARRAY_BUFFER)) &&
        check_gl_logger_call("glBindVertexArray unbind", gore::bind_vertex_array_log,
                             static_cast<GLuint>(0)) &&
        check_gl_logger_call("glEnableVertexAttribArray position",
                             gore::enable_vertex_attrib_array_log, static_cast<GLuint>(0)) &&
        check_gl_logger_call("glVertexAttribPointer position", gore::vertex_attrib_log,
                             static_cast<GLuint>(0), static_cast<GLint>(2),
                             static_cast<GLenum>(GL_FLOAT), static_cast<GLboolean>(GL_FALSE),
                             static_cast<GLsizei>(sizeof(gore::vec2)),
                             static_cast<const void*>(nullptr));
    const bool draw_data_passed = check_gl_logger_call(
        "glDrawArrays font", gore::draw_arrays_log,
        static_cast<GLenum>(GL_LINES), static_cast<GLint>(0));
    const bool raster_draw_data_passed = check_gl_logger_call(
        "glDrawArrays raster font", gore::draw_arrays_log,
        static_cast<GLenum>(GL_TRIANGLES), static_cast<GLint>(0));
    print_gl_logger_output();
    std::printf("font outline color: %s\n", outline_found ? "PASS" : "FAIL");
    std::printf("font raster color: %s\n", raster_found ? "PASS" : "FAIL");
    std::printf("font batching: %s\n", batching_passed ? "PASS" : "FAIL");
    return outline_found && raster_found && batching_passed && gl_logging_passed && call_data_passed &&
           draw_data_passed && raster_draw_data_passed ? 0 : 1;
}
#endif