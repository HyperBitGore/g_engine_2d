// Image test: loads PNG and BMP images, draws them static, rotated, and
// UV-cropped. Press 1-4 to toggle which image is in focus. Esc to quit.
// Bottom row tests multi-texture unit batch rendering: 7 textures are
// queued with addImageVertex then flushed in a single drawBuffer() call.
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/img_loading/image_loader.hpp"
#include <string>

static const uint32_t W = 800;
static const uint32_t H = 600;

gore::g_engine_2d eng("Image Test", W, H, 0, gore::LogType::NONE);

static std::unique_ptr<gore::image_renderer>    img_r;
static std::unique_ptr<gore::grayscalerenderer> gray_r;

static gore::IMG img_png;
static gore::IMG img_bmp;
static gore::IMG img_gray;
static gore::IMG img_blank;
static gore::IMG img_palette;
static gore::IMG img_enemy;
static gore::IMG img_test;

static float angle   = 0.0f;
static double cooldown = 0.0;
static int focused = 0; // which image is highlighted

static const char* labels[] = {
    "1: PNG (Bliss)",
    "2: BMP (Bliss)",
    "3: Grayscale PNG",
    "4: Programmatic blank",
};

void render() {
    eng.enable(GL_BLEND);

    // static PNG
    img_r->drawImage(img_png,  {20.0f,  20.0f},  {180.0f, 135.0f});
    // rotating BMP
    img_r->drawImageRotated(img_bmp, {300.0f, 20.0f},  {180.0f, 135.0f}, angle);
    // grayscale PNG
    gray_r->drawImage(img_gray, {560.0f, 20.0f},  {180.0f, 135.0f});
    // blank programmatic image (red gradient drawn at startup)
    img_r->drawImage(img_blank, {20.0f, 200.0f}, {180.0f, 180.0f});
    // UV crop (top-left quarter of the PNG)
    img_r->drawImage(img_png, {240.0f, 200.0f}, {180.0f, 135.0f}, {0.0f, 0.0f, 0.5f, 0.5f});

    // multi-texture unit batch: 7 different textures queued together and
    // flushed in a single drawBuffer() call to stress the texture unit map
    // row 1
    img_r->addImageVertex(img_png->tex,     {20.0f,  400.0f}, {150.0f, 112.0f});
    img_r->addImageVertex(img_bmp->tex,     {185.0f, 400.0f}, {150.0f, 112.0f});
    img_r->addImageVertex(img_blank->tex,   {350.0f, 400.0f}, {112.0f, 112.0f});
    img_r->addImageVertex(img_palette->tex, {475.0f, 400.0f}, {150.0f, 112.0f});
    // row 2
    img_r->addImageVertex(img_enemy->tex,   {20.0f,  525.0f}, {112.0f,  60.0f});
    img_r->addImageVertex(img_test->tex,    {145.0f, 525.0f}, {112.0f, 112.0f});
    img_r->addImageVertex(img_gray->tex,    {270.0f, 525.0f}, {150.0f, 112.0f});
    img_r->drawBuffer();

    eng.disable(GL_BLEND);
}

int main() {
    img_png     = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP).png");
    img_bmp     = gore::imageloader::loadBMP("resources/Bliss_(Windows_XP).bmp");
    img_gray    = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_grayscale16.png");
    img_palette = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_pallette.png");
    img_enemy   = gore::imageloader::loadPNG("resources/enem2_1.png");
    img_test    = gore::imageloader::loadPNG("resources/test.png");

    // programmatic blank — fill with a red-to-blue horizontal gradient
    img_blank = gore::imageloader::createBlank(64, 64, 4);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint8_t r = (uint8_t)(x * 4);
            uint8_t b = (uint8_t)(255 - x * 4);
            gore::imageloader::setPixel(img_blank, x, y, r, 0, b, 255);
        }
    }
    gore::imageloader::createTexture(img_blank, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    gore::imageloader::updateIMG(img_blank);

    img_r  = gore::createImageRenderer(W, H);
    gray_r = gore::image_renderer::create<gore::grayscalerenderer>(W, H);
    eng.addRenderer(img_r.get(),  false, false, false);
    eng.addRenderer(gray_r.get(), false, false, false);

    eng.setRenderFunction(render);

    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();
        cooldown += dt;

        angle += (float)(dt * 1.2f);
        if (angle > 2.0f * (float)M_PI) angle -= 2.0f * (float)M_PI;

        if (cooldown >= 0.25 && eng.getKeyReleased(g_Escape)) break;
    }

    return 0;
}
