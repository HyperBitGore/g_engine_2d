// basic first person dungeon crawling to test matrice projection
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/wireframe_renderer.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/rendering/camera.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <memory>

uint32_t W = 1024;
uint32_t H = 768;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, PRIMITIVE_COMPONENT, gore::LogType::NONE);
std::unique_ptr<gore::wireframe_renderer> wireframe_r = nullptr;
std::unique_ptr<gore::threedeerender> three_d = nullptr;
std::vector<gore::vec3> penger;
gore::camera cam({0.0, 0.0, 5.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0});

void render () {
    three_d->addVertexs(penger);
    three_d->drawBuffer();
}
gore::vec2 last_mouse = {(float)W / 2.0f, (float)H / 2.0f};
bool view_dirty = false;
const float sensitivity = 0.002f;
bool capture = false;
gore::vec2 center = {(float)W / 2.0f, (float)H / 2.0f};
void mouseMove () {
    if (!capture) {
        return;
    }
    gore::vec2 p = eng.getMousePos();
    std::cout << "x: " <<  std::abs(p.x - center.x) << " y: " << std::abs(p.y - center.y) << "\n";
    if (std::abs(p.x - center.x) <= 1 && std::abs(p.y - center.y) < 1) {
        std::cout << "center\n";
        return;
    }
    // std::cout << p.x << ", " << p.y << "\n";
    float dx = p.x - last_mouse.x;
    float dy = p.y - last_mouse.y;
    if (std::abs(dx) > 1.0f || std::abs(dy) > 1.0f) {
        cam.setYaw(cam.getYaw() + dx * sensitivity);
        cam.setPitch(cam.getPitch() - dy * sensitivity);
        cam.updateFront();
        view_dirty = true;
    }
    last_mouse = {p.x, p.y};
    // last_mouse = {(float)W / 2.0f, (float)H / 2.0f};
}
void resize(uint32_t w, uint32_t h) {
    W = w;
    H = h;
    center = {(float)W / 2.0f, (float)H / 2.0f};
}

int main () {
    penger = gore::model_loader::loadObj("resources/penger.obj");
    eng.setRenderFunction(render);
    eng.setWindowResize(resize);
    eng.setFrameLimit(60);
    eng.toggleFrameLimitActive();
    eng.setMouseMoveFunction(mouseMove);
    wireframe_r = gore::wireframe_renderer::create(W, H);
    three_d = gore::threedeerender::create(W, H);
    three_d->updateView(cam.pos, cam.pos + cam.front, cam.up);


    bool running = true;
    while (running) {
        // Sample mouse BEFORE updateWindow so we read position prior to the warp-to-center
        gore::vec2 cur_mouse = eng.getMousePos(true);
        running = eng.updateWindow();
        if (!running) break;

        eng.updateInputState();
        double delta = eng.getDelta();
        float speed = 2.5f * (float)delta;

        // Escape toggles mouse-look on/off
        if (eng.getKeyReleased(g_Escape)) {
            eng.toggleMouseCapture(true);
            // Snapshot current position so the first delta is zero
            last_mouse = cur_mouse;
            capture = !capture;
        }

        if (eng.getKeyDown(g_w)) { cam.moveForward(speed);  view_dirty = true; }
        else if (eng.getKeyDown(g_s)) { cam.moveBackward(speed); view_dirty = true; }
        if (eng.getKeyDown(g_a)) { cam.moveLeft(speed);   view_dirty = true; }
        else if (eng.getKeyDown(g_d)) { cam.moveRight(speed);  view_dirty = true; }

        if (view_dirty) {
            three_d->updateView(cam.pos, cam.pos + cam.front, cam.up);
            view_dirty = false;
        }
    }
}