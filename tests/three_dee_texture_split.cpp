// three_dee_texture_split: verifies that threedeerender::addModel() automatically
// calls drawBuffer() (splitting the draw call) when the texture unit limit is reached.
//
// The test creates (max_texture_units * 2) uniquely-coloured 8x8 textures, each
// attached to a separate textured cube model. All models are submitted via addModel()
// in a single frame. The renderer must internally flush mid-batch when the unit count
// reaches the hardware limit (twice in total), and then the explicit drawBuffer() at
// the end of the frame flushes the remainder. All coloured cubes must appear on
// screen for the test to pass.
//
// Console output reports the detected texture-unit limit and the model indices at which
// the auto-flushes are expected. Press Esc to exit.

#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/three_dee_renderer.hpp"
#include "../include/g_engine/img_loading/image_loader.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

static const uint32_t W = 900;
static const uint32_t H = 600;

gore::g_engine_2d eng("3D Texture Unit Split Test", W, H, 0, gore::LogType::NONE);

static std::unique_ptr<gore::threedeerender> three_r;
static std::vector<gore::model> models;
static int max_units  = 0;
static int model_count = 0;

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

// Returns the 12 triangles (6 faces × 2 triangles) of an axis-aligned unit cube
// centred at (cx, cy, cz) with half-side length s. All faces share material 0.
static std::vector<gore::model_face> make_cube(float cx, float cy, float cz, float s) {
    gore::vec3 ftl = {cx-s, cy+s, cz+s};
    gore::vec3 ftr = {cx+s, cy+s, cz+s};
    gore::vec3 fbr = {cx+s, cy-s, cz+s};
    gore::vec3 fbl = {cx-s, cy-s, cz+s};
    gore::vec3 btl = {cx-s, cy+s, cz-s};
    gore::vec3 btr = {cx+s, cy+s, cz-s};
    gore::vec3 bbr = {cx+s, cy-s, cz-s};
    gore::vec3 bbl = {cx-s, cy-s, cz-s};

    std::vector<gore::model_face> faces;
    auto quad = [&](gore::vec3 a, gore::vec3 b, gore::vec3 c, gore::vec3 d) {
        gore::model_face f1, f2;
        f1.p1=a; f1.p2=b; f1.p3=c; f1.uv1={0,0}; f1.uv2={1,0}; f1.uv3={1,1}; f1.material_index=0;
        f2.p1=a; f2.p2=c; f2.p3=d; f2.uv1={0,0}; f2.uv2={1,1}; f2.uv3={0,1}; f2.material_index=0;
        faces.push_back(f1);
        faces.push_back(f2);
    };
    quad(ftl, ftr, fbr, fbl); // front
    quad(btr, btl, bbl, bbr); // back
    quad(btl, ftl, fbl, bbl); // left
    quad(ftr, btr, bbr, fbr); // right
    quad(btl, btr, ftr, ftl); // top
    quad(fbl, fbr, bbr, bbl); // bottom
    return faces;
}

void render() {
    for (auto& m : models) {
        three_r->addModel(m);
    }
    three_r->drawBuffer();
}

int main() {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);
    model_count = max_units * 2;

    printf("GL_MAX_TEXTURE_IMAGE_UNITS = %d\n", max_units);
    printf("Creating %d textured cubes — auto-flush expected at models %d and %d\n",
           model_count, max_units - 1, model_count - 1);

    // Grid layout: cubes spread across the view.
    // Camera is at (0,0,5) looking at origin; FOV 45°, aspect 1.5.
    const int   COLS   = 8;
    const float S      = 0.12f; // cube half-side
    const float STEP   = 0.38f;
    const float orig_x = -(COLS / 2.0f) * STEP + STEP * 0.5f;
    const float orig_y =  1.2f;

    three_r = gore::threedeerender::create(W, H);

    models.reserve(model_count);
    for (int idx = 0; idx < model_count; idx++) {
        uint8_t r, g, b;
        hue_to_rgb((float)idx / (float)model_count, r, g, b);

        gore::IMG img = gore::imageloader::createBlank(8, 8, 4);
        for (int py = 0; py < 8; py++)
            for (int px = 0; px < 8; px++)
                gore::imageloader::setPixel(img, px, py, r, g, b, 255);
        gore::imageloader::createTexture(img, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        gore::imageloader::updateIMG(img);

        int   col = idx % COLS;
        int   row = idx / COLS;
        float cx  = orig_x + col * STEP;
        float cy  = orig_y - row * STEP;

        gore::model m(make_cube(cx, cy, 0.0f, S));
        // Tilt each cube so three faces are visible
        m.rotate({0.0f, 1.0f, 0.0f}, 0.5f);
        m.rotate({1.0f, 0.0f, 0.0f}, 0.3f);
        m.addImageMaterial(std::move(img), "tex_" + std::to_string(idx));
        models.push_back(std::move(m));
    }

    eng.addRenderer(three_r.get(), false, false, false);
    eng.setRenderFunction(render);

    while (eng.updateWindow()) {
        eng.updateInputState();
        if (eng.getKeyReleased(g_Escape)) break;
    }

    return 0;
}
