// Test maintain_renderer_view component mask functionality
// Tests MAINTAIN_ASPECT_RATIO_COMPONENT
// Renders a grid to verify the component is working correctly
// Esc - Quit

#include "../include/g_engine/g_engine_2d.hpp"
#include <memory>

static const uint32_t W = 1024;
static const uint32_t H = 768;

gore::g_engine_2d eng("Maintain Renderer View Test", W, H, MAINTAIN_ASPECT_RATIO_COMPONENT, gore::LogType::NONE, "");

// Renderers
static std::unique_ptr<gore::linerenderer> line_r;

void render_grid() {
    line_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    // Draw a grid pattern to visualize the aspect ratio maintenance
    line_r->addLine({0.0f, 100.0f}, {W * 1.0f, 100.0f});
    line_r->addLine({0.0f, 200.0f}, {W * 1.0f, 200.0f});
    line_r->addLine({0.0f, 300.0f}, {W * 1.0f, 300.0f});
    line_r->addLine({0.0f, 400.0f}, {W * 1.0f, 400.0f});
    line_r->addLine({0.0f, 500.0f}, {W * 1.0f, 500.0f});
    line_r->addLine({0.0f, 600.0f}, {W * 1.0f, 600.0f});

    line_r->addLine({100.0f, 0.0f}, {100.0f, H * 1.0f});
    line_r->addLine({200.0f, 0.0f}, {200.0f, H * 1.0f});
    line_r->addLine({300.0f, 0.0f}, {300.0f, H * 1.0f});
    line_r->addLine({400.0f, 0.0f}, {400.0f, H * 1.0f});
    line_r->addLine({500.0f, 0.0f}, {500.0f, H * 1.0f});
    line_r->addLine({600.0f, 0.0f}, {600.0f, H * 1.0f});
    line_r->addLine({700.0f, 0.0f}, {700.0f, H * 1.0f});
    line_r->addLine({800.0f, 0.0f}, {800.0f, H * 1.0f});
    line_r->addLine({900.0f, 0.0f}, {900.0f, H * 1.0f});

    line_r->drawBuffer();
}

void render() {
    //eng.enable(GL_BLEND);
    //eng.disable(GL_DEPTH_TEST);

    render_grid();

    //eng.disable(GL_BLEND);
}

int main() {
    line_r = gore::linerenderer::create(W, H);

    eng.setRenderFunction(render);
    eng.addRenderer(line_r.get(), true, false, false);

    eng.logger->log("=== Maintain Renderer View Component Test ===");
    eng.logger->log("Testing MAINTAIN_ASPECT_RATIO_COMPONENT");
    eng.logger->log("Rendering a grid to verify component functionality");
    eng.logger->log("Esc - Quit");
    eng.logger->flush();

    while (eng.updateWindow()) {
        eng.updateInputState();

        if (eng.getKeyReleased(g_Escape)) {
            break;
        }
    }

    eng.logger->log("Test completed.");
    eng.logger->flush();

    return 0;
}
