// basic first person dungeon crawling to test matrice projection
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/wireframe_renderer.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <memory>

static const uint32_t W = 1024;
static const uint32_t H = 768;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, PRIMITIVE_COMPONENT, gore::LogType::NONE);
std::unique_ptr<gore::wireframe_renderer> wireframe_r = nullptr;
std::unique_ptr<gore::threedeerender> three_d = nullptr;
std::vector<gore::vec3> penger;
gore::vec3 position = { 0.0, 0.0, 5.0 };

void render () {
    // three_d->addTriangle({0.0f, 0.0f, 2.0f}, {1.0f, 0.0f, 2.0f}, {0.0f, 1.0f, 2.0f});
    three_d->addVertexs(penger);
    three_d->drawBuffer();
}


int main () {
    penger = gore::model_loader::loadObj("resources/penger.obj");
    eng.setRenderFunction(render);
    eng.setFrameLimit(60);
    eng.toggleFrameLimitActive();
    wireframe_r = gore::wireframe_renderer::create(W, H);
    three_d = gore::threedeerender::create(W, H);
    double movement_count = 0.0f;
    while(eng.updateWindow()) {
        eng.updateInputState();
        double delta = eng.getDelta();
        movement_count += delta;
        if (movement_count > 0.1f) {
            bool moved = false;
            if (eng.getKeyDown(g_w)) {
                position.z += 0.1f;
                moved = true;
            } else if (eng.getKeyDown(g_s)) {
                position.z -= 0.1f;
                moved = true;
            }
            if (eng.getKeyDown(g_a)) {
                position.x -= 0.1f;
                moved = true;
            } else if (eng.getKeyDown(g_d)) {
                position.x += 0.1f;
                moved = true;
            }

            if (moved) {
                three_d->updateView(position, {0, 0, 0}, {0, 1.0f, 0.0f});
            }
            movement_count = 0;
        }
    }
}