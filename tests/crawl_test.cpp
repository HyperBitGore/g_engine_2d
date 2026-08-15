// basic first person dungeon crawling to test matrice projection
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/wireframe_renderer.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/rendering/camera.hpp"
#include "../include/g_engine/rendering/billboard.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <memory>

uint32_t W = 1024;
uint32_t H = 768;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, 0, gore::LogType::NONE);
std::unique_ptr<gore::wireframe_renderer> wireframe_r = nullptr;
std::unique_ptr<gore::threedeerender> three_d = nullptr;
std::unique_ptr<gore::image_renderer> image = nullptr;
std::vector<gore::vec3> penger;
gore::model peng;
gore::model cube_tex;
gore::model duck;
gore::camera cam({0.0, 0.0, 5.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 0.0});
gore::vec3 peng_pos = {0.0, 2.0, 0.0};
std::unique_ptr<gore::billboard> board = nullptr;
std::unique_ptr<gore::billboard> static_board = nullptr;
double duck_switch = 0;
bool duck_rend = false;
void render () {
    eng.enable(GL_CULL_FACE);
    three_d->addModel(peng);
    gore::IMG& ipg = cube_tex.getImage(0);
    three_d->addModel(cube_tex);
    if (duck_rend) {
        three_d->addModel(duck);
    }
    three_d->addBillboard(*board, cam);
    three_d->addBillboard(*static_board, cam);
    three_d->drawBuffer();
    eng.disable(GL_CULL_FACE);
    image->drawImage(ipg, {100.0f, 100.0f}, {50.0f, 50.0f});
}
gore::vec2 last_mouse = {(float)W / 2.0f, (float)H / 2.0f};
bool view_dirty = false;
const float sensitivity = 0.002f;
bool capture = false;
gore::vec2 center = {(float)W / 2.0f, (float)H / 2.0f};
void mouseMove () {
    if (!capture) return;
     float dx = std::trunc(eng.getMousePos().x - (W / 2.0f));  // delta from center
     float dy = std::trunc(eng.getMousePos().y - (H / 2.0f));
     cam.setYaw(cam.getYaw() + dx * sensitivity);
     cam.setPitch(cam.getPitch() + dy * sensitivity);
     cam.update();
     view_dirty = true;
}
void resize(uint32_t w, uint32_t h) {
    W = w;
    H = h;
    center = {(float)W / 2.0f, (float)H / 2.0f};
}
int main () {
    cube_tex = gore::model_loader::loadObj("resources/cube-tex.obj");
    peng = gore::model_loader::loadObj("resources/penger.obj");
    duck = gore::model_loader::loadGltf("resources/duck/Duck.gltf");
    duck.scale({0.01f, 0.01f, 0.01f});
    duck.translate({0.0f, 0.0f, 1.0f});
    GLint max_ssbo_size;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_ssbo_size);
    std::cout << "Max ssbo: " << max_ssbo_size << "\n";
    peng.translate({0.0, 2.0, 0.0});
    penger = peng.getPositions();
    eng.setRenderFunction(render);
    eng.setWindowResize(resize);
    //eng.setFrameLimit(60);
    //eng.toggleFrameLimitActive();
    eng.setMouseMoveFunction(mouseMove);
    wireframe_r = gore::wireframe_renderer::create(W, H);
    three_d = gore::createThreeDeeRenderer(W, H);
    image = gore::createImageRenderer(W, H);
    eng.addRenderer(wireframe_r.get(), false, false, false);
    eng.addRenderer(three_d.get(),     false, false, true);
    three_d->updateView(cam.getPos(), cam.getPos() + cam.getFront(), cam.getUp());

    gore::IMG board_img = gore::imageloader::loadPNG("resources/penger.png");
    gore::IMG test_static = gore::imageloader::loadPNG("resources/test.png");
    board = std::make_unique<gore::billboard>(
        gore::vec3{-3.0f, 1.0f, 0.0f},
        gore::vec2{2.0f, 2.0f},
        std::move(board_img),
        gore::billboard_type::DISTANCE_ADJUST_SIZE
    );
    static_board = std::make_unique<gore::billboard>(
        gore::vec3{5.0f, 1.0f, 0.0f},
        gore::vec2{0.2f, 0.2f},
        std::move(test_static),
        gore::billboard_type::FIXED_SIZE
    );

    eng.setClearColor({0.0, 0.5, 1.0, 1.0});
    bool running = true;
    double peng_count = 0.0;
    double peng_change = -0.1;
    float cube_angle = 0.0f;
    const float cube_rot_speed = 1.0f; // radians per second
    while (running) {
        // Sample mouse BEFORE updateWindow so we read position prior to the warp-to-center
        gore::vec2 cur_mouse = eng.getMousePos(true);
        running = eng.updateWindow();
        if (!running) break;

        eng.updateInputState();
        double delta = eng.getDelta();
        float speed = 2.5f * (float)delta;
        duck_switch += delta;
        cube_angle += cube_rot_speed * (float)delta;
        cube_tex.resetMatrix();
        cube_tex.rotate({0.0f, 1.0f, 0.0f}, cube_angle);

        peng_count += delta;
        if (peng_count > 0.1) {
            peng_count = 0;
            if (peng_pos.y + peng_change < 0.0) {
                peng_change = 0.05;
            } else if (peng_pos.y + peng_change > 2.5) {
                peng_change = -0.05;
            }
            peng_pos.y += peng_change;
            peng.translate(peng_pos);
        }
        if (duck_switch > 0.5) {
            duck_rend = !duck_rend;
            duck_switch = 0;
        }
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
        if (eng.getKeyDown(g_q)) { cam.tiltLeft (speed); view_dirty = true; }
        else if (eng.getKeyDown(g_e)) { cam.tiltRight (speed); view_dirty = true; }

        if (view_dirty) {
            cam.update();
            three_d->updateView(cam.getPos(), cam.getPos() + cam.getFront(), cam.getUp());
            view_dirty = false;
        }
    }
}