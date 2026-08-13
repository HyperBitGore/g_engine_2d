// Visual test for adding, removing, and updating instance_render instances.
// Press Escape to exit.

#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/rendering/camera.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include "../include/g_engine/file_loading/font_loader.hpp"
#include "../include/g_engine/rendering/font_renderer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

static const uint32_t W = 900;
static const uint32_t H = 600;
static uint32_t window_width = W;
static uint32_t window_height = H;

struct live_instance {
    gore::model* model;
    int32_t index;
    gore::matrix transform;
    gore::vec3 origin;
};

static gore::matrix make_transform(float x, float y, float z, float scale) {
    gore::matrix transform = gore::matrix::generateIdentity(4, 4);
    transform = transform.translate({x, y, z});
    return transform.scale({scale, scale, scale});
}

static std::unique_ptr<gore::instance_render> renderer;
static std::vector<live_instance> live_instances;
static gore::model* cube_model = nullptr;
static gore::model* penguin_model = nullptr;
static gore::model* duck_model = nullptr;
static float elapsed = 0.0f;
static float next_mutation = 0.1f;
static std::mt19937 rng(0xC0FFEEu);
static gore::camera camera(
    {0.0f, 0.0f, 5.0f},
    {0.0f, 0.0f, -1.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}
);
static bool view_dirty = false;
static bool capture = false;
static const float sensitivity = 0.002f;
static std::unique_ptr<gore::fontrenderer> font_r;
static gore::font fps_font;
static std::string fps_text = "FPS: --";
static uint32_t dpi = 96;

static void render() {
    renderer->drawBuffer();

    font_r->setColor({1.0f, 1.0f, 0.2f, 1.0f});
    font_r->drawText(fps_text, &fps_font, 10.0f,
                     static_cast<float>(window_height) - 30.0f, 18, dpi);
}

static void spawn_instance(gore::model* model, float x, float y, float z, float scale) {
    gore::matrix transform = make_transform(x, y, z, scale);
    int32_t index = renderer->addModelInstance(model, transform);
    if (index >= 0) {
        live_instances.push_back({
            model,
            index,
            std::move(transform),
            {x, y, z}
        });
    }
}

static float model_scale(gore::model* model) {
    if (model == cube_model) return 1.0f;
    if (model == penguin_model) return 0.75f;
    return 0.01f;
}

static void mutate_instances(float delta) {
    elapsed += delta;

    // Move a rotating sample of live instances without changing geometry.
    for (size_t i = 0; i < live_instances.size(); ++i) {
        live_instance& instance = live_instances[i];
        float phase = elapsed + static_cast<float>(i) * 0.03f;
        float amplitude = 0.75f + static_cast<float>(i % 5) * 0.25f;
        instance.transform = make_transform(
            instance.origin.x + std::sin(phase) * amplitude,
            instance.origin.y,
            instance.origin.z,
            model_scale(instance.model)
        );
        renderer->updateModelInstance(instance.model, instance.index, instance.transform);
    }

    // Remove 100 random instances in descending slot order so index shifts
    // do not invalidate the slots that are removed later.
    if (elapsed >= next_mutation) {
        std::shuffle(live_instances.begin(), live_instances.end(), rng);
        std::vector<live_instance> removed(
            live_instances.begin(),
            live_instances.begin() + 100
        );
        std::sort(removed.begin(), removed.end(),
            [](const live_instance& lhs, const live_instance& rhs) {
                return lhs.index > rhs.index;
            });

        for (const live_instance& instance : removed) {
            renderer->removeModelInstance(instance.model, instance.index);
            live_instances.erase(
                std::find_if(live_instances.begin(), live_instances.end(),
                    [&](const live_instance& current) {
                        return current.model == instance.model
                            && current.index == instance.index;
                    })
            );
            for (live_instance& current : live_instances) {
                if (current.index > instance.index) {
                    --current.index;
                }
            }
        }
        // Add 100 replacement instances with random model types and positions.
        std::uniform_int_distribution<int> type_dist(0, 2);
        std::uniform_real_distribution<float> position_dist(-50.0f, 50.0f);
        for (int i = 0; i < 100; ++i) {
            const int type = type_dist(rng);
            gore::model* model = type == 0
                ? cube_model
                : (type == 1 ? penguin_model : duck_model);
            spawn_instance(
                model,
                position_dist(rng),
                position_dist(rng),
                0.0f,
                model_scale(model)
            );
        }
        next_mutation += 0.1f;
    }
}

static void mouseMove(gore::g_engine_2d& eng) {
    if (!capture) return;

    float dx = std::trunc(eng.getMousePos().x - (window_width / 2.0f));
    float dy = std::trunc(eng.getMousePos().y - (window_height / 2.0f));
    camera.setYaw(camera.getYaw() + dx * sensitivity);
    camera.setPitch(camera.getPitch() + dy * sensitivity);
    camera.update();
    view_dirty = true;
}

int main() {
    gore::g_engine_2d eng("Instance Renderer Mutation Test", W, H, 0, gore::LogType::NONE);
    renderer = gore::instance_render::create(W, H);
    font_r = gore::fontrenderer::create(W, H);
    fps_font = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 32, 127);
    font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    dpi = eng.getDPI();

    gore::model cube = gore::model_loader::loadObj("resources/cube-tex.obj");
    gore::model penguin = gore::model_loader::loadObj("resources/penger.obj");
    gore::model duck = gore::model_loader::loadGltf("resources/duck/Duck.gltf");
    cube_model = &cube;
    penguin_model = &penguin;
    duck_model = &duck;

    renderer->addModelData(cube_model, 334);
    renderer->addModelData(penguin_model, 333);
    renderer->addModelData(duck_model, 333);
    eng.enable(GL_CULL_FACE);
    // Register each model as a contiguous range. Removing duck instances
    // from the end then leaves cube and penguin indices unchanged.
    for (int i = 0; i < 334; ++i) {
        spawn_instance(cube_model, (i % 34) * 2.5f - 42.0f,
                       ((float)i / 34) * 2.5f - 12.0f, 0.0f,
                       model_scale(cube_model));
    }
    for (int i = 0; i < 333; ++i) {
        spawn_instance(penguin_model, (i % 34) * 2.5f - 42.0f,
                       ((float)i / 34) * 2.5f + 15.0f, 0.0f,
                       model_scale(penguin_model));
    }
    for (int i = 0; i < 333; ++i) {
        spawn_instance(duck_model, (i % 34) * 2.5f - 42.0f,
                       ((float)i / 34) * 2.5f + 42.0f, 0.0f,
                       model_scale(duck_model));
    }

    eng.addRenderer(renderer.get(), false, false, true);
    eng.addRenderer(font_r.get(), false, false, false);
    eng.setRenderFunction(render);
    eng.setWindowResize([](uint32_t width, uint32_t height) {
        window_width = width;
        window_height = height;
    });
    eng.setMouseMoveFunction([&eng]() {
        mouseMove(eng);
    });
    renderer->updateView(
        camera.getPos(),
        camera.getPos() + camera.getFront(),
        camera.getUp()
    );
    eng.setClearColor({0.05f, 0.05f, 0.08f, 1.0f});

    double fps_timer = 0.0;
    uint32_t frame_count = 0;
    while (eng.updateWindow()) {
        double delta = eng.getDelta();
        fps_timer += delta;
        ++frame_count;
        if (fps_timer >= 0.1) {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "FPS: %.1f",
                          frame_count / fps_timer);
            fps_text = buffer;
            fps_timer = 0.0;
            frame_count = 0;
        }
        mutate_instances(static_cast<float>(delta));
        eng.updateInputState();
        float speed = 10.5f * static_cast<float>(delta);

        if (eng.getKeyReleased(g_Escape)) {
            eng.toggleMouseCapture(true);
            capture = !capture;
        }

        if (eng.getKeyDown(g_w)) { camera.moveForward(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_s)) { camera.moveBackward(speed); view_dirty = true; }
        if (eng.getKeyDown(g_a)) { camera.moveLeft(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_d)) { camera.moveRight(speed); view_dirty = true; }
        if (eng.getKeyDown(g_q)) { camera.tiltLeft(speed); view_dirty = true; }
        else if (eng.getKeyDown(g_e)) { camera.tiltRight(speed); view_dirty = true; }

        if (view_dirty) {
            camera.update();
            renderer->updateView(
                camera.getPos(),
                camera.getPos() + camera.getFront(),
                camera.getUp()
            );
            view_dirty = false;
        }
    }
}
