#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/rendering/three_dee_renderer.hpp"
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
    std::printf("three-dee triangle color: %s\n", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}
