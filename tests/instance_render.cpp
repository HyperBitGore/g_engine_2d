// Visual test for instance_render and multi-draw instancing.
// Press Escape to exit.

#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/rendering/camera.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <cmath>
#include <memory>
#include <vector>

static uint32_t W = 900;
static uint32_t H = 600;

static gore::index_buffer<gore::model_vertex> makeCube() {
    const gore::vec3 p[] = {
        {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, 0.5f},
        { 0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f},
        {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}
    };

    const gore::vec3 normals[] = {
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f},
        {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
    };

    const uint32_t faces[][4] = {
        {0, 1, 2, 3}, {5, 4, 7, 6},
        {4, 0, 3, 7}, {1, 5, 6, 2},
        {4, 5, 1, 0}, {3, 2, 6, 7}
    };

    std::vector<gore::model_vertex> vertices;
    std::vector<GLuint> indices;
    vertices.reserve(24);
    indices.reserve(36);

    for (size_t face = 0; face < 6; ++face) {
        const size_t base = vertices.size();
        const uint32_t* corners = faces[face];
        const gore::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f},
                                  {1.0f, 1.0f}, {0.0f, 1.0f}};
        for (size_t corner = 0; corner < 4; ++corner) {
            vertices.push_back({
                p[corners[corner]],
                normals[face],
                uvs[corner],
                -1
            });
        }
        for (uint32_t index : {0u, 1u, 2u, 0u, 2u, 3u}) {
            indices.push_back(static_cast<GLuint>(base + index));
        }
    }

    return gore::index_buffer<gore::model_vertex>(
        std::move(vertices), std::move(indices));
}

static gore::matrix at(float x, float y, float z) {
    gore::matrix transform = gore::matrix::generateIdentity(4, 4);
    return transform.translate({x, y, z});
}

static std::unique_ptr<gore::instance_render> instance_r;
struct model_instance {
    gore::model* model;
    gore::matrix transform;
};
static std::vector<model_instance> instances;
static gore::camera cam(
    {0.0f, 0.0f, 5.0f},
    {0.0f, 0.0f, -1.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}
);
static gore::vec2 last_mouse = {(float)W / 2.0f, (float)H / 2.0f};
static bool view_dirty = false;
static bool capture = false;
static const float sensitivity = 0.002f;

static void render() {
    instance_r->drawBuffer();
}

static void mouseMove(gore::g_engine_2d& eng) {
    if (!capture) return;

    float dx = std::trunc(eng.getMousePos().x - (W / 2.0f));
    float dy = std::trunc(eng.getMousePos().y - (H / 2.0f));
    cam.setYaw(cam.getYaw() + dx * sensitivity);
    cam.setPitch(cam.getPitch() - dy * sensitivity);
    cam.update();
    view_dirty = true;
}

int main() {
    gore::g_engine_2d eng("Instance Renderer Test", W, H, 0, gore::LogType::NONE);
    instance_r = gore::instance_render::create(W, H);

    gore::model cube(
        makeCube(),
        gore::ModelType::OBJ
    );
    gore::model cube2 = gore::model_loader::loadObj("resources/cube-tex.obj");
    gore::model penguin = gore::model_loader::loadObj("resources/penger.obj");
    gore::model duck = gore::model_loader::loadGltf("resources/duck/Duck.gltf");

    // Upload each model's static geometry once. Instance transforms are then
    // registered once and retained by the renderer across frames.
    constexpr int GRID_SIZE = 100;
    constexpr float SPACING = 3.5f;
    instance_r->addModelData(&cube, 3334);
    instance_r->addModelData(&penguin, 3333);
    instance_r->addModelData(&duck, 3333);

    // One shared mesh per model, submitted as 10,000 persistent instances.
    instances.reserve(GRID_SIZE * GRID_SIZE);
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            const int model_index = (x + y) % 3;
            gore::model* model = model_index == 0
                ? &cube
                : (model_index == 1 ? &penguin : &duck);
            gore::matrix transform = at(
                (static_cast<float>(x) - GRID_SIZE / 2.0f) * SPACING,
                (static_cast<float>(y) - GRID_SIZE / 2.0f) * SPACING,
                0.0f
            );
            if (model == &penguin) {
                transform = transform.scale({0.75f, 0.75f, 0.75f});
            } else if (model == &duck) {
                transform = transform.scale({0.01f, 0.01f, 0.01f});
            }
            instances.push_back({model, std::move(transform)});
        }
    }
    for (const model_instance& instance : instances) {
        instance_r->addModelInstance(instance.model, instance.transform);
    }

    eng.addRenderer(instance_r.get(), false, false, true);
    eng.setRenderFunction(render);
    eng.setWindowResize([](uint32_t w, uint32_t h) {
        W = w;
        H = h;
    });
    eng.setMouseMoveFunction([&eng]() {
        mouseMove(eng);
    });
    instance_r->updateView(cam.getPos(), cam.getPos() + cam.getFront(), cam.getUp());
    eng.setClearColor({0.05f, 0.05f, 0.08f, 1.0f});

    while (eng.updateWindow()) {
        gore::vec2 cur_mouse = eng.getMousePos(true);
        eng.updateInputState();
        double delta = eng.getDelta();
        float speed = 2.5f * (float)delta;

        if (eng.getKeyReleased(g_Escape)) {
            eng.toggleMouseCapture(true);
            last_mouse = cur_mouse;
            capture = !capture;
        }

        if (eng.getKeyDown(g_w)) { cam.moveForward(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_s)) { cam.moveBackward(speed); view_dirty = true; }
        if (eng.getKeyDown(g_a)) { cam.moveLeft(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_d)) { cam.moveRight(speed); view_dirty = true; }
        if (eng.getKeyDown(g_q)) { cam.tiltLeft(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_e)) { cam.tiltRight(speed); view_dirty = true; }

        if (view_dirty) {
            cam.update();
            instance_r->updateView(
                cam.getPos(),
                cam.getPos() + cam.getFront(),
                cam.getUp()
            );
            view_dirty = false;
        }
    }

    return 0;
}
