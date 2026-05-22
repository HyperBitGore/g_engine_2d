#include "../include/g_engine/g_engine_2d.hpp"
static const uint32_t W = 800;
static const uint32_t H = 600;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, PRIMITIVE_COMPONENT, gore::LogType::NONE);

class SoftwareRenderer {
    private:

    public:
    
};


gore::vec2 projectPoint (gore::vec3 p) {
    return { p.x / p.z, p.y / p.z};
}

float z = 1.0f;
void render () {
    eng.triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
    eng.triangle_r->drawQuad(projectPoint({250.0f, 250.0f, z}), 10.0f, 10.0f);
}

int main () {
    std::cout << "Hello world" << std::endl;
    eng.setRenderFunction(render);
    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        z += 1 * dt;
        eng.updateInputState();
    }
    return 0;
} 