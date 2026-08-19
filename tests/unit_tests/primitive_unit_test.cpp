#include "../../include/g_engine/g_engine_2d.hpp"
#include "gl_logger_output.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#if defined(_UNIT_TEST_)
namespace {

constexpr uint32_t WIDTH = 64;
constexpr uint32_t HEIGHT = 64;
constexpr uint8_t COLOR_TOLERANCE = 2;

struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

pixel sample(gore::drawpass& pass, GLint x, GLint y) {
    pixel result{};
    pass.bind();
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &result);
    pass.unbind();
    return result;
}

bool close(uint8_t actual, uint8_t expected) {
    return std::abs(static_cast<int>(actual) - static_cast<int>(expected)) <= COLOR_TOLERANCE;
}

bool is_color(pixel actual, pixel expected) {
    return close(actual.r, expected.r) &&
           close(actual.g, expected.g) &&
           close(actual.b, expected.b) &&
           close(actual.a, expected.a);
}

int failures = 0;

void check_color(const char* name, pixel actual, pixel expected) {
    if (is_color(actual, expected)) {
        std::printf("  PASS  %s\n", name);
    } else {
        std::printf("  FAIL  %s: got (%u, %u, %u, %u), expected (%u, %u, %u, %u)\n",
                    name,
                    actual.r, actual.g, actual.b, actual.a,
                    expected.r, expected.g, expected.b, expected.a);
        ++failures;
    }
}

void check_color_near(const char* name, gore::drawpass& pass, GLint x, GLint y, pixel expected) {
    for (GLint sample_y = y - 2; sample_y <= y + 2; ++sample_y) {
        for (GLint sample_x = x - 2; sample_x <= x + 2; ++sample_x) {
            if (is_color(sample(pass, sample_x, sample_y), expected)) {
                std::printf("  PASS  %s\n", name);
                return;
            }
        }
    }
    std::printf("  FAIL  %s: no matching sampled point color\n", name);
    ++failures;
}

} // namespace

int main() {
    gore::g_engine_2d engine("Primitive Unit Test", WIDTH, HEIGHT, 0, gore::LogType::NONE);
    auto triangles = gore::trianglerenderer::create(WIDTH, HEIGHT);
    auto lines = gore::linerenderer::create(WIDTH, HEIGHT);
    auto points = gore::pointrenderer::create(WIDTH, HEIGHT);
    gore::drawpass pass(WIDTH, HEIGHT, GL_COLOR_ATTACHMENT0);

    engine.setRenderFunction([&]() {
        pass.bind();
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        triangles->setColor({1.0f, 0.0f, 0.0f, 1.0f});
        triangles->drawTriangle({4.0f, 4.0f}, {20.0f, 4.0f}, {4.0f, 20.0f});

        triangles->setColor({0.0f, 1.0f, 0.0f, 1.0f});
        triangles->drawQuad({28.0f, 4.0f}, 12.0f, 12.0f);

        lines->setColor({0.0f, 0.0f, 1.0f, 1.0f});
        lines->setLineWidth(3.0f);
        lines->drawLine({4.0f, 28.0f}, {20.0f, 28.0f});

        glPointSize(5.0f);
        points->setColor({1.0f, 1.0f, 0.0f, 1.0f});
        points->drawPoint({32.0f, 32.0f});

        pass.unbind();
    });

    if (!engine.updateWindow()) {
        std::printf("  FAIL  render frame\n");
        return 1;
    }
    check_color("triangle color", sample(pass, 8, 56), {255, 0, 0, 255});
    check_color("quad color", sample(pass, 34, 54), {0, 255, 0, 255});
    check_color("line color", sample(pass, 12, 36), {0, 0, 255, 255});
    check_color_near("point color", pass, 32, 32, {255, 255, 0, 255});
    check_color("clear color outside primitives", sample(pass, 56, 56), {0, 0, 0, 255});

    failures += !check_gl_logger("glDrawArrays", gore::draw_arrays_log);
    failures += !check_gl_logger("glBindBuffer", gore::bind_buffer_log);
    failures += !check_gl_logger("glBufferData", gore::buffer_data_log);
    failures += !check_gl_logger("glBindVertexArray", gore::bind_vertex_array_log);
    failures += !check_gl_logger("glEnableVertexAttribArray", gore::enable_vertex_attrib_array_log);
    failures += !check_gl_logger("glVertexAttribPointer", gore::vertex_attrib_log);
    failures += !check_gl_logger("glVertexAttribIPointer", gore::vertex_attrib_i_log);
    failures += !check_gl_logger("glBindBufferBase", gore::bind_buffer_base_log);
    failures += !check_gl_logger_call(
        "glBindBuffer array", gore::bind_buffer_log,
        static_cast<GLenum>(GL_ARRAY_BUFFER));
    failures += !check_gl_logger_call(
        "glBufferData array", gore::buffer_data_log,
        static_cast<GLenum>(GL_ARRAY_BUFFER), static_cast<GLsizeiptr>(sizeof(gore::vec2) * 3));
    failures += !check_gl_logger_call(
        "glBindVertexArray unbind", gore::bind_vertex_array_log,
        static_cast<GLuint>(0));
    failures += !check_gl_logger_call(
        "glEnableVertexAttribArray position", gore::enable_vertex_attrib_array_log,
        static_cast<GLuint>(0));
    failures += !check_gl_logger_call(
        "glVertexAttribPointer position", gore::vertex_attrib_log,
        static_cast<GLuint>(0), static_cast<GLint>(2), static_cast<GLenum>(GL_FLOAT),
        static_cast<GLboolean>(GL_FALSE), static_cast<GLsizei>(sizeof(gore::vec2)),
        static_cast<const void*>(nullptr));
    failures += !check_gl_logger_call(
        "glVertexAttribIPointer texture", gore::vertex_attrib_i_log,
        static_cast<GLuint>(4), static_cast<GLint>(1), static_cast<GLenum>(GL_UNSIGNED_INT));
    failures += !check_gl_logger_call(
        "glBindBufferBase shader storage", gore::bind_buffer_base_log,
        static_cast<GLenum>(GL_SHADER_STORAGE_BUFFER), static_cast<GLuint>(0));
    failures += !check_gl_logger_call(
        "glDrawArrays triangle", gore::draw_arrays_log,
        static_cast<GLenum>(GL_TRIANGLES), static_cast<GLint>(0), static_cast<GLsizei>(3));
    print_gl_logger_output();
    std::printf("\n%d failed\n", failures);
    return failures == 0 ? 0 : 1;
}
#endif