// Text test: renders text using both the vector fontrenderer (outline) and
// the raster fontraster (bitmap) paths at multiple sizes. Esc to quit.
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/file_loading/font_loader.hpp"
#include "../include/g_engine/rendering/font_renderer.hpp"

static const uint32_t W = 900;
static const uint32_t H = 700;

gore::g_engine_2d eng("Text Test", W, H, FONT_COMPONENT | IMAGE_COMPONENT, gore::LogType::NONE);

static gore::font f_mono;
static gore::font f_sans;
static double cooldown = 0.0;

void render() {
    // --- vector (outline) renderer ---
    eng.font_renderer->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    eng.font_renderer->drawText("Vector outline — 16pt", &f_mono, 20.0f, 40.0f,  16, eng.getDPI());
    eng.font_renderer->drawText("Vector outline — 24pt", &f_mono, 20.0f, 80.0f,  24, eng.getDPI());
    eng.font_renderer->drawText("Vector outline — 32pt", &f_sans, 20.0f, 130.0f, 32, eng.getDPI());
    eng.font_renderer->drawText("The quick brown fox 0123456789", &f_sans, 20.0f, 180.0f, 20, eng.getDPI());

    eng.font_renderer->setColor({1.0f, 0.5f, 0.2f, 1.0f});
    eng.font_renderer->drawText("Colored vector text", &f_mono, 20.0f, 230.0f, 28, eng.getDPI());

    // u16string with extended chars
    std::u16string u16str = {u'H', u'e', u'l', u'l', u'o', u' ', 0x00C9, u'!'};
    eng.font_renderer->setColor({0.4f, 1.0f, 0.6f, 1.0f});
    eng.font_renderer->drawText(u16str, &f_sans, 20.0f, 275.0f, 28, eng.getDPI());

    // --- raster renderer ---
    eng.enable(GL_BLEND);
    gore::fontraster::drawRasterText(&f_mono, eng.img_r.get(), "Raster — 16pt", 20.0f, 350.0f, 16, eng.getDPI());
    gore::fontraster::drawRasterText(&f_mono, eng.img_r.get(), "Raster — 32pt", 20.0f, 400.0f, 32, eng.getDPI());
    gore::fontraster::drawRasterText(&f_sans, eng.img_r.get(), "Sans raster — 24pt", 20.0f, 460.0f, 24, eng.getDPI());
    gore::fontraster::drawRasterText(&f_sans, eng.img_r.get(),
        "abcdefghijklmnopqrstuvwxyz 0123456789", 20.0f, 520.0f, 18, eng.getDPI());
    gore::fontraster::drawRasterText(&f_mono, eng.img_r.get(),
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20.0f, 570.0f, 18, eng.getDPI());
    eng.disable(GL_BLEND);
}

int main() {
    f_mono = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 0, 735);
    f_sans = gore::fontloader::loadFont("resources/OpenSans-Regular.ttf", 32, 127);

    gore::fontraster::rasterizeFont(&f_mono, 64, eng.getDPI(), 0xFFFFFFFF, 32, 127);
    gore::fontraster::rasterizeFont(&f_sans, 64, eng.getDPI(), 0xFFFFFFFF, 32, 127);

    eng.setRenderFunction(render);

    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();
        cooldown += dt;

        if (cooldown >= 0.25 && eng.getKeyReleased(g_Escape)) break;
    }

    return 0;
}
