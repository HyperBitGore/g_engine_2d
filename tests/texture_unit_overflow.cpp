// texture_unit_overflow: verifies that imagerenderer::drawBuffer() is called
// automatically by getTextureUnit() when the texture unit limit is reached.
//
// The test creates (max_texture_units * 2) uniquely-colored 32x32 textures and
// queues them all with addImageVertex() in a single frame. The renderer must
// internally flush mid-batch twice as the limit is crossed twice, and then the
// explicit drawBuffer() at the end of the frame flushes the remainder. All
// tiles must appear on screen with their correct colors for the test to pass.
//
// Console output reports the detected texture unit limit and the index at which
// the auto-flush is expected to trigger. Press Esc to exit.
#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/img_loading/image_loader.hpp"
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <vector>

static const uint32_t W = 900;
static const uint32_t H = 600;

gore::g_engine_2d eng("Texture Unit Overflow Test", W, H, 0, gore::LogType::NONE);

static std::unique_ptr<gore::image_renderer> img_r;
static std::vector<gore::IMG> textures;
static int max_units = 0;
static int tile_count = 0;

// Tile layout
static const int TILE_W  = 48;
static const int TILE_H  = 48;
static const int TILE_GAP = 6;
static const int COLS    = 10;

// Convert hue [0,1) to an RGB triple, fully saturated and valued.
static void hue_to_rgb(float h, uint8_t& r, uint8_t& g, uint8_t& b) {
    float s = 1.0f, v = 0.85f;
    int   i = (int)(h * 6.0f);
    float f = h * 6.0f - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    float fr, fg, fb;
    switch (i % 6) {
        case 0: fr=v; fg=t; fb=p; break;
        case 1: fr=q; fg=v; fb=p; break;
        case 2: fr=p; fg=v; fb=t; break;
        case 3: fr=p; fg=q; fb=v; break;
        case 4: fr=t; fg=p; fb=v; break;
        default: fr=v; fg=p; fb=q; break;
    }
    r = (uint8_t)(fr * 255.0f);
    g = (uint8_t)(fg * 255.0f);
    b = (uint8_t)(fb * 255.0f);
}

void render() {
    eng.enable(GL_BLEND);

    for (int i = 0; i < tile_count; i++) {
        int col = i % COLS;
        int row = i / COLS;
        float x = (float)(20 + col * (TILE_W + TILE_GAP));
        float y = (float)(20 + row * (TILE_H + TILE_GAP));

        // Each addImageVertex call with a brand-new texture consumes one unit.
        // When current_unit + 1 >= texture_units the renderer auto-flushes.
        img_r->addImageVertex(textures[i]->tex, {x, y}, {(float)TILE_W, (float)TILE_H});
    }

    // Flush whatever remains after any mid-batch auto-flush(es).
    img_r->drawBuffer();

    eng.disable(GL_BLEND);
}

int main() {
    // Query the hardware limit before creating the renderer so we know how
    // many textures are needed to trigger the overflow path.
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);
    tile_count = max_units * 2; // crosses the limit twice, exercising two auto-flushes

    printf("GL_MAX_TEXTURE_IMAGE_UNITS = %d\n", max_units);
    printf("Creating %d textures — auto-flush expected after queuing textures %d and %d\n",
           tile_count, max_units - 1, max_units * 2 - 1);

    // Create one 32x32 solid-color texture per tile.
    for (int i = 0; i < tile_count; i++) {
        uint8_t r, g, b;
        hue_to_rgb((float)i / (float)tile_count, r, g, b);

        gore::IMG img = gore::imageloader::createBlank(32, 32, 4);
        for (int py = 0; py < 32; py++)
            for (int px = 0; px < 32; px++)
                gore::imageloader::setPixel(img, px, py, r, g, b, 255);

        gore::imageloader::createTexture(img, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        gore::imageloader::updateIMG(img);
        textures.push_back(std::move(img));
    }

    img_r = gore::createImageRenderer(W, H);
    eng.addRenderer(img_r.get(), false, false, false);
    eng.setRenderFunction(render);

    while (eng.updateWindow()) {
        eng.updateInputState();
        if (eng.getKeyReleased(g_Escape)) break;
    }

    return 0;
}
