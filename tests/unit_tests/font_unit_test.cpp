#include "../../include/g_engine/g_engine_2d.hpp"
#include "../../include/g_engine/file_loading/font_loader.hpp"
#include "../../include/g_engine/rendering/font_renderer.hpp"
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
    gore::g_engine_2d engine("Font Renderer Unit Test", size, size, 0, gore::LogType::NONE);
    gore::font font = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 0, 735);
    auto renderer = gore::fontrenderer::create(size, size);
    gore::drawpass pass(size, size, GL_COLOR_ATTACHMENT0);

    engine.setRenderFunction([&] {
        pass.bind();
        glViewport(0, 0, size, size);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer->setColor({1, 0, 0, 1});
        renderer->drawText("A", &font, 16, 32, 32, engine.getDPI());
        pass.unbind();
    });

    if (!engine.updateWindow()) return 1;

    bool found = false;
    for (GLint y = 0; y < static_cast<GLint>(size) && !found; ++y) {
        for (GLint x = 0; x < static_cast<GLint>(size); ++x) {
            const pixel value = sample(pass, x, y);
            if (value.r > 200 && value.g < 10 && value.b < 10 && value.a > 0) {
                found = true;
                break;
            }
        }
    }
    std::printf("font outline color: %s\n", found ? "PASS" : "FAIL");
    return found ? 0 : 1;
}
