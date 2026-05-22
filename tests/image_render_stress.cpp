// Stress test for image_renderer: queues large numbers of image draw calls
// in a single batch per frame to measure throughput and catch buffer growth
// / reallocation bugs.
//
// Controls:
//   Up/Down  — increase / decrease sprite count by 100
//   Esc      — quit
//
// Each frame, all sprites are added via addImageVertex() and flushed with a
// single drawBuffer() call.  Sprites cycle through all loaded textures so
// every texture unit slot gets exercised under load.
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/img_loading/image_loader.hpp"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

static const uint32_t W = 800;
static const uint32_t H = 600;

static gore::font f_mono;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, IMAGE_COMPONENT | FONT_COMPONENT | PRIMITIVE_COMPONENT, gore::LogType::NONE);

// all textures used in the batch
static gore::IMG img_png;
static gore::IMG img_bmp;
static gore::IMG img_palette;
static gore::IMG img_enemy;
static gore::IMG img_test;
static gore::IMG img_blank;

static const int  SPRITE_W   = 32;
static const int  SPRITE_H   = 32;
static const int  TEX_COUNT  = 6;
static const int  COUNT_STEP = 100;
static const int  COUNT_MAX  = 10000;
static const int  COUNT_MIN  = 100;

static int    sprite_count = 500;
static double cooldown     = 0.0;

struct Sprite {
    float x, y;
    float vx, vy;
    int   tex_idx;
};
static std::vector<Sprite> sprites;
static GLuint tex_pool[TEX_COUNT];

static void init_sprites(int count) {
    sprites.resize(count);
    for (int i = 0; i < count; i++) {
        sprites[i].x       = fmodf((float)(i * 37), (float)(W - SPRITE_W));
        sprites[i].y       = fmodf((float)(i * 19), (float)(H - SPRITE_H));
        sprites[i].vx      = 30.0f + fmodf((float)(i * 7),  60.0f);
        sprites[i].vy      = 20.0f + fmodf((float)(i * 11), 50.0f);
        sprites[i].tex_idx = i % TEX_COUNT;
    }
}

static void update_sprites(double dt) {
    for (auto& s : sprites) {
        s.x += s.vx * (float)dt;
        s.y += s.vy * (float)dt;
        if (s.x < 0.0f || s.x + SPRITE_W > (float)W) {
            s.vx = -s.vx;
            s.x  = s.x < 0.0f ? 0.0f : (float)(W - SPRITE_W);
        }
        if (s.y < 0.0f || s.y + SPRITE_H > (float)H) {
            s.vy = -s.vy;
            s.y  = s.y < 0.0f ? 0.0f : (float)(H - SPRITE_H);
        }
    }
}
static std::pair<double,double> last_frames = {0, 0};
void render() {
    // queue every sprite — all different textures, single draw call
    for (const auto& s : sprites)
        eng.img_r->addImageVertex(tex_pool[s.tex_idx],
                                  {s.x, s.y},
                                  {(float)SPRITE_W, (float)SPRITE_H});
    eng.img_r->drawBuffer();

    char hud[128];
    snprintf(hud, sizeof(hud), "Sprites: %d   FPS: %.0f   Avg: %.2f ms",
             sprite_count, last_frames.first, last_frames.second);
    eng.font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    eng.font_r->drawText(hud, &f_mono, 10.0f, 30.0f, 20, eng.getDPI());
}

int main() {
    img_png     = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP).png");
    img_bmp     = gore::imageloader::loadBMP("resources/Bliss_(Windows_XP).bmp");
    img_palette = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_pallette.png");
    img_enemy   = gore::imageloader::loadPNG("resources/enem2_1.png");
    img_test    = gore::imageloader::loadPNG("resources/test.png");
    f_mono = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 0, 735);

    img_blank = gore::imageloader::createBlank(32, 32, 4);
    for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++)
            gore::imageloader::setPixel(img_blank, x, y,
                                        (uint8_t)(x * 8), (uint8_t)(y * 8), 128, 255);
    gore::imageloader::createTexture(img_blank, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    gore::imageloader::updateIMG(img_blank);

    tex_pool[0] = img_png->tex;
    tex_pool[1] = img_bmp->tex;
    tex_pool[2] = img_palette->tex;
    tex_pool[3] = img_enemy->tex;
    tex_pool[4] = img_test->tex;
    tex_pool[5] = img_blank->tex;

    init_sprites(sprite_count);
    eng.setRenderFunction(render);

    char title[64];
    snprintf(title, sizeof(title), "Image Renderer Stress — %d sprites", sprite_count);
    eng.setWindowTitle(title);
    double second_acc = 0.0;
    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        eng.updateInputState();
        cooldown += dt;
        second_acc += dt;

        if (second_acc >= 1.0) {
            last_frames = eng.getFrames();
            second_acc = 0.0;
        }
        update_sprites(dt);

        if (cooldown >= 0.15) {
            bool changed = false;
            if (eng.getKeyReleased(g_UpArrow) && sprite_count < COUNT_MAX) {
                sprite_count += COUNT_STEP;
                changed = true;
            }
            if (eng.getKeyReleased(g_DownArrow) && sprite_count > COUNT_MIN) {
                sprite_count -= COUNT_STEP;
                changed = true;
            }
            if (changed) {
                init_sprites(sprite_count);
                cooldown = 0.0;
                snprintf(title, sizeof(title),
                         "Image Renderer Stress — %d sprites", sprite_count);
                eng.setWindowTitle(title);
            }
            if (eng.getKeyReleased(g_Escape)) break;
        }
    }

    return 0;
}
