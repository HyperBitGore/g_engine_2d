// Dual engine test: constructs two g_engine_2d instances to observe how the
// engine behaves with more than one window/context alive at once.
//
// Expectation notes:
//  - Each constructor creates its own window + GL context and makes it current,
//    so the SECOND engine's context is current after both are constructed.
//  - Renderers bind VAOs/VBOs against whichever context is current at creation
//    time, so each engine's renderers are created immediately after that engine.
//  - Without a makeCurrent() between the two updateWindow() calls, both render
//    functions execute against the last-current context. Watch which window
//    actually displays content.
// Esc (in either window) quits.
#include "../include/g_engine/g_engine_2d.hpp"
#include <cstdio>
#include <memory>

static const uint32_t W = 640;
static const uint32_t H = 480;

static std::unique_ptr<gore::g_engine_2d> eng1;
static std::unique_ptr<gore::g_engine_2d> eng2;

static std::unique_ptr<gore::trianglerenderer> tri1;
static std::unique_ptr<gore::trianglerenderer> tri2;

static void render1() {
    // red quad, top-left
    tri1->setColor({1.0f, 0.2f, 0.2f, 1.0f});
    tri1->drawQuad({W - 190.0f, H - 190.0f}, 150.0f, 150.0f);
}

static void render2() {
    // blue quad, bottom-right
    tri2->setColor({0.2f, 0.4f, 1.0f, 1.0f});
    tri2->drawQuad({W - 190.0f, H - 190.0f}, 150.0f, 150.0f);
}

int main() {
    printf("constructing engine 1...\n");
    eng1 = std::make_unique<gore::g_engine_2d>("Dual Engine - 1", W, H, MAINTAIN_ASPECT_RATIO_COMPONENT, gore::LogType::CONSOLE);
    // create eng1's renderers while its context is (presumably) current
    tri1 = gore::trianglerenderer::create(W, H);
    eng1->addRenderer(tri1.get(), true, false, false);
    eng1->setRenderFunction(render1);
    printf("engine 1 up\n");

    printf("constructing engine 2...\n");
    eng2 = std::make_unique<gore::g_engine_2d>("Dual Engine - 2", W, H, MAINTAIN_ASPECT_RATIO_COMPONENT, gore::LogType::CONSOLE);
    tri2 = gore::trianglerenderer::create(W, H);
    eng2->addRenderer(tri2.get(), true, false, false);
    eng2->setRenderFunction(render2);
    printf("engine 2 up\n");

#if defined(__unix__)
    printf("current GLX context after both constructions: %p\n", (void*)glXGetCurrentContext());
#endif

    unsigned long long frame = 0;
    bool run1 = true, run2 = true;
    double cooldown = 0.0;

    while (run1 || run2) {
        if (run1) {
            eng1->makeContextCurrent();
            run1 = eng1->updateWindow();
            eng1->updateInputState();
        }
        if (run2) {
            eng2->makeContextCurrent();
            run2 = eng2->updateWindow();
            eng2->updateInputState();
        }

        cooldown += (run1 ? eng1->getDelta() : eng2->getDelta());
        if (cooldown >= 0.25 &&
            ((run1 && eng1->getKeyReleased(g_Escape)) ||
             (run2 && eng2->getKeyReleased(g_Escape)))) {
            break;
        }

        if (frame % 300 == 0) {
#if defined(__unix__)
            printf("frame %llu | run1=%d run2=%d | current ctx=%p drawable=%lu\n",
                   frame, run1, run2, (void*)glXGetCurrentContext(),
                   (unsigned long)glXGetCurrentDrawable());
#else
            printf("frame %llu | run1=%d run2=%d\n", frame, run1, run2);
#endif
        }
        frame++;
    }

    printf("shutting down (frame %llu)\n", frame);
    // destroy in reverse order of construction
    tri2.reset();
    eng2.reset();
    tri1.reset();
    eng1.reset();
    printf("clean exit\n");
    return 0;
}
