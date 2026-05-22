// Primitives test: draws every primitive type provided by the split renderers.
// Left/right arrow keys scroll the view. Esc to quit.
#include "../include/g_engine/g_engine_2d.hpp"
#include <cmath>

static const uint32_t W = 800;
static const uint32_t H = 600;

gore::g_engine_2d eng("Primitives Test", W, H, PRIMITIVE_COMPONENT, gore::LogType::NONE);

static float angle   = 0.0f;
static float cx      = 0.0f;
static double cooldown = 0.0;

void render() {
    // --- triangles ---
    eng.triangle_r->setColor({1.0f, 0.3f, 0.1f, 1.0f});
    eng.triangle_r->drawTriangle({40.0f, 100.0f}, {80.0f, 40.0f}, {120.0f, 100.0f});

    eng.triangle_r->setColor({0.2f, 0.8f, 0.3f, 1.0f});
    eng.triangle_r->addTriangle({160.0f, 100.0f}, {200.0f, 40.0f}, {240.0f, 100.0f});
    eng.triangle_r->addTriangle({170.0f, 105.0f}, {210.0f, 165.0f}, {250.0f, 105.0f});
    eng.triangle_r->drawBuffer();

    // --- quads ---
    eng.triangle_r->setColor({0.2f, 0.5f, 1.0f, 1.0f});
    eng.triangle_r->drawQuad({280.0f, 40.0f}, 100.0f, 70.0f);

    eng.triangle_r->setColor({1.0f, 1.0f, 0.2f, 1.0f});
    eng.triangle_r->addQuad({400.0f, 40.0f}, 50.0f, 50.0f);
    eng.triangle_r->addQuad({460.0f, 40.0f}, 50.0f, 50.0f);
    eng.triangle_r->drawBuffer();

    // --- filled circle ---
    eng.triangle_r->setColor({0.9f, 0.2f, 0.9f, 1.0f});
    eng.triangle_r->drawCircleFilled({600.0f, 75.0f}, 50.0f);

    // --- circle outline ---
    eng.line_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    eng.line_r->drawCircleOutline({700.0f, 75.0f}, 45.0f, 36);

    // --- points ---
    eng.point_r->setColor({1.0f, 1.0f, 0.0f, 1.0f});
    eng.point_r->drawPoint({80.0f, 200.0f});
    for (int i = 0; i < 20; i++) {
        eng.point_r->addPoint({120.0f + i * 15.0f, 200.0f + sinf(i * 0.5f + angle) * 30.0f});
    }
    eng.point_r->drawBuffer();

    // --- lines ---
    eng.line_r->setColor({0.0f, 1.0f, 1.0f, 1.0f});
    eng.line_r->drawLine({40.0f, 280.0f}, {760.0f, 280.0f});

    eng.line_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
    eng.line_r->setLineWidth(3.0f);
    for (int i = 0; i < 8; i++) {
        float x = 50.0f + i * 90.0f;
        eng.line_r->addLine({x, 320.0f}, {x + 60.0f, 380.0f});
    }
    eng.line_r->drawBuffer();
    eng.line_r->setLineWidth(1.0f);

    // --- linear bezier ---
    eng.line_r->setColor({0.8f, 0.8f, 0.2f, 1.0f});
    eng.line_r->drawLinearBezier({40.0f, 430.0f}, {400.0f, 430.0f});

    // --- quadratic bezier ---
    eng.line_r->setColor({0.3f, 0.9f, 0.6f, 1.0f});
    eng.line_r->drawQuadraticBezier({40.0f, 480.0f}, {200.0f + cx, 400.0f}, {400.0f, 480.0f}, 40);

    // --- cubic bezier ---
    eng.line_r->setColor({1.0f, 0.3f, 0.7f, 1.0f});
    eng.line_r->drawCubicBezier({40.0f, 550.0f}, {160.0f, 440.0f}, {300.0f, 580.0f}, {500.0f, 500.0f}, 40);
}

int main() {
    eng.setRenderFunction(render);

    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();
        cooldown += dt;

        angle += (float)(dt * 2.0f);

        cx = sinf(angle) * 80.0f;

        if (cooldown >= 0.25 && eng.getKeyReleased(g_Escape)) break;
    }

    return 0;
}
