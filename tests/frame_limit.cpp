#include "../include/g_engine/g_engine_2d.hpp"
#include <cstdint>
#include <string>

// Frame limit test: renders a FPS counter on screen and lets you cycle through
// preset limits with 1/2/3 to verify the limiter is working. Press Escape to quit.
//   1 -> 30 fps
//   2 -> 60 fps
//   3 -> 120 fps

static const uint32_t W = 640;
static const uint32_t H = 480;

gore::g_engine_2d eng("Frame Limit Test", W, H, FONT_COMPONENT, gore::LogType::NONE);

static double second_acc = 0.0;
static double cooldown = 0.0;
static std::pair<double,double> last_frames = {0, 0};
static std::string status_msg = "60 fps limit active";

void render() {
    eng.font_renderer->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    eng.font_renderer->drawText(status_msg, nullptr, 20.0f, H - 40.0f, 16, eng.getDPI());

    std::string fps_str = "FPS: " + std::to_string((int)last_frames.first)
                        + "  avg: " + std::to_string(last_frames.second).substr(0, 5) + " ms";
    eng.font_renderer->drawText(fps_str, nullptr, 20.0f, H - 70.0f, 16, eng.getDPI());

    eng.font_renderer->setColor({0.6f, 0.6f, 0.6f, 1.0f});
    eng.font_renderer->drawText("1=30fps  2=60fps  3=120fps  Esc=quit", nullptr, 20.0f, 20.0f, 14, eng.getDPI());
}

int main() {
    eng.setRenderFunction(render);
    eng.toggleFrameLimitActive();
    eng.setFrameLimit(60);

    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();

        second_acc += dt;
        cooldown += dt;

        if (second_acc >= 1.0) {
            last_frames = eng.getFrames();
            second_acc = 0.0;
        }

        if (cooldown >= 0.3) {
            if (eng.getKeyReleased(g_1)) {
                eng.setFrameLimit(30);
                status_msg = "30 fps limit active";
                cooldown = 0.0;
            } else if (eng.getKeyReleased(g_2)) {
                eng.setFrameLimit(60);
                status_msg = "60 fps limit active";
                cooldown = 0.0;
            } else if (eng.getKeyReleased(g_3)) {
                eng.setFrameLimit(120);
                status_msg = "120 fps limit active";
                cooldown = 0.0;
            }
        }

        if (eng.getKeyReleased(g_Escape)) {
            break;
        }
    }

    return 0;
}
