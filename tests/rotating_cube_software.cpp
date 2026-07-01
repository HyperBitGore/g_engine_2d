#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/rendering/wireframe_renderer.hpp"
#include "../include/g_engine/file_loading/model_loading/model_loader.hpp"
#include <memory>
static const uint32_t W = 800;
static const uint32_t H = 600;

gore::g_engine_2d eng("Image Renderer Stress Test", W, H, 0, gore::LogType::NONE);
std::unique_ptr<gore::wireframe_renderer> wireframe_r = nullptr;
gore::model penger;

struct SoftwareRendererPoint {
    float x;
    float y;
};

class SoftwareRenderer : public gore::renderer<SoftwareRenderer, SoftwareRendererPoint> {
    private:
    	friend class gore::renderer<SoftwareRenderer, SoftwareRendererPoint>;
        void shader_setup() override {
            draw_arrays_mode = GL_LINES;
            shader.bind();
			shader.setuniform("set_color", { 1.0f, 1.0f, 1.0f, 1.0f});
            glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SoftwareRendererPoint), (void*)0);
        }
        SoftwareRenderer (size_t width, size_t height) : gore::renderer<SoftwareRenderer, SoftwareRendererPoint> ("resources/software.vs", "resources/software.fs", width, height, true) {

        }
    public:
        void setColor (gore::vec4 color) {
            shader.setuniform("set_color", color);
        }
        void addLine (gore::vec2 a, gore::vec2 b) {
            vertexs.push_back({a.x, a.y});
            vertexs.push_back({b.x, b.y});
        }
};


gore::vec2 projectPoint (gore::vec3 p) {
    return { p.x / p.z, p.y / p.z};
}

gore::vec3 rotatePoint (gore::vec3 p, gore::axis a, float angle) {
    gore::vec3 np = p;
    switch (a) {
        case gore::axis::x:
            np.x = p.x;
            np.y = p.y * cosf(angle) - p.z * sinf(angle);
            np.z = p.y*sinf(angle)+p.z*cosf(angle);
        break;
        case gore::axis::y:
            np.x = p.x * cosf(angle) + p.z * sinf(angle);
            np.y = p.y;
            np.z = -p.x*sinf(angle)+p.z*cosf(angle);
        break;
        case gore::axis::z:
            np.x = p.x*cosf(angle)-p.y*sinf(angle);
            np.y = p.x * sinf(angle) + p.y * cosf(angle);
            np.z = p.z;
        break;
        case gore::axis::nil:
        break;
    }
    return np;
}
float angle = 0;
float triangle_angle = 0;
std::vector<gore::vec3> points = {
    {0.25f, 0.25f, 1.5f},
    {-0.25f, 0.25f, 1.5},
    {0.25f, -0.25f, 1.5f},
    {-0.25f, -0.25f, 1.5f},
    
    {0.25f, 0.25f, 2.0f},
    {-0.25f, 0.25f, 2.0f},
    {0.25f, -0.25f, 2.0f},
    {-0.25f, -0.25f, 2.0f},
};
std::vector<gore::vec3> points_wire_cube = {
    {0.5f, 0.5f, 1.5f},
    {-0.5f, 0.5f, 1.5},
    {0.5f, -0.5f, 1.5f},
    {-0.5f, -0.5f, 1.5f},
    
    {0.5f, 0.5f, 2.0f},
    {-0.5f, 0.5f, 2.0f},
    {0.5f, -0.5f, 2.0f},
    {-0.5f, -0.5f, 2.0f},
};
static const gore::vec3 tl_cube[8] = {
    {-0.95f,  0.85f, 1.35f}, // 0 front TL
    {-0.65f,  0.85f, 1.35f}, // 1 front TR
    {-0.65f,  0.55f, 1.35f}, // 2 front BR
    {-0.95f,  0.55f, 1.35f}, // 3 front BL
    {-0.95f,  0.85f, 1.65f}, // 4 back TL
    {-0.65f,  0.85f, 1.65f}, // 5 back TR
    {-0.65f,  0.55f, 1.65f}, // 6 back BR
    {-0.95f,  0.55f, 1.65f}, // 7 back BL
};
gore::vec3 tl_center = {-0.80f, 0.70f, 1.50f};
gore::vec3 center = {0.0f, 0.0f, 1.75f};
std::unique_ptr<SoftwareRenderer> software_r = nullptr;
gore::axis current_axis = gore::axis::y;
gore::axis triangle_axis = gore::axis::z;
float cloud_time = 0.0f;
float cloud_angle = 0.0f;
gore::vec3 cloud_center = {0.0f, 0.0f, 1.8f};
void render () {
    // rotate all 8 points first
    gore::vec2 p[8];
    for (size_t i = 0; i < points.size(); i++) {
        gore::vec3 rotated = rotatePoint(points[i] - center, current_axis, angle) + center;
        p[i] = projectPoint(rotated);
    }
    // front face (0-3), back face (4-7), connecting edges
    software_r->addLine(p[0], p[1]); software_r->addLine(p[0], p[2]);
    software_r->addLine(p[1], p[3]); software_r->addLine(p[2], p[3]);
    software_r->addLine(p[4], p[5]); software_r->addLine(p[4], p[6]);
    software_r->addLine(p[5], p[7]); software_r->addLine(p[6], p[7]);
    software_r->addLine(p[0], p[4]); software_r->addLine(p[1], p[5]);
    software_r->addLine(p[2], p[6]); software_r->addLine(p[3], p[7]);
    software_r->drawBuffer();
    wireframe_r->addLine(points_wire_cube[0], points_wire_cube[1], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[0], points_wire_cube[2], angle, current_axis, center);
    wireframe_r->addLine(points_wire_cube[1], points_wire_cube[3], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[2], points_wire_cube[3], angle, current_axis, center);
    wireframe_r->addLine(points_wire_cube[4], points_wire_cube[5], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[4], points_wire_cube[6], angle, current_axis, center);
    wireframe_r->addLine(points_wire_cube[5], points_wire_cube[7], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[6], points_wire_cube[7], angle, current_axis, center);
    wireframe_r->addLine(points_wire_cube[0], points_wire_cube[4], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[1], points_wire_cube[5], angle, current_axis, center);
    wireframe_r->addLine(points_wire_cube[2], points_wire_cube[6], angle, current_axis, center); 
    wireframe_r->addLine(points_wire_cube[3], points_wire_cube[7], angle, current_axis, center);
    // static triangle cube in top left
    const gore::vec3* c = tl_cube;
    wireframe_r->addTriangle(c[0], c[1], c[2], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[0], c[2], c[3], triangle_angle, triangle_axis, tl_center); // front
    wireframe_r->addTriangle(c[4], c[5], c[6], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[4], c[6], c[7], triangle_angle, triangle_axis, tl_center); // back
    wireframe_r->addTriangle(c[0], c[1], c[5], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[0], c[5], c[4], triangle_angle, triangle_axis, tl_center); // top
    wireframe_r->addTriangle(c[3], c[2], c[6], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[3], c[6], c[7], triangle_angle, triangle_axis, tl_center); // bottom
    wireframe_r->addTriangle(c[0], c[3], c[7], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[0], c[7], c[4], triangle_angle, triangle_axis, tl_center); // left
    wireframe_r->addTriangle(c[1], c[2], c[6], triangle_angle, triangle_axis, tl_center); wireframe_r->addTriangle(c[1], c[6], c[5], triangle_angle, triangle_axis, tl_center); // right
    // rotating undulating point cloud — each point drawn as a small cross
    constexpr int CW = 9, CH = 9;
    constexpr float step = 0.11f;
    constexpr float e = 0.014f;
    for (int xi = 0; xi < CW; xi++) {
        for (int zi = 0; zi < CH; zi++) {
            float x = (xi - (CW - 1) * 0.5f) * step;
            float z = cloud_center.z - (CH - 1) * step * 0.5f + zi * step;
            float y = sinf(x * 10.0f + cloud_time) * cosf(z * 8.0f + cloud_time * 0.8f) * 0.18f;
            gore::vec3 p = {x, y, z};
            wireframe_r->addLine({p.x - e, p.y, p.z}, {p.x + e, p.y, p.z}, cloud_angle, gore::axis::y, cloud_center);
            wireframe_r->addLine({p.x, p.y - e, p.z}, {p.x, p.y + e, p.z}, cloud_angle, gore::axis::y, cloud_center);
        }
    }
    wireframe_r->drawBuffer();
}

int main () {
    software_r = SoftwareRenderer::create(W, H);
    penger = gore::model_loader::loadObj("resources/penger.obj");
    wireframe_r = gore::wireframe_renderer::create(W, H);
    eng.addRenderer(software_r.get(),  false, false, false);
    eng.addRenderer(wireframe_r.get(), false, false, false);
    eng.setRenderFunction(render);
    eng.setFrameLimit(60);
    eng.toggleFrameLimitActive();
    float angle_change = 0;
    float angle_triangle_change = 0;
    while (eng.updateWindow()) {
        double dt = eng.getDelta();
        angle += M_PI*dt;
        triangle_angle += M_PI * dt;
        cloud_time += (float)(dt * 2.0);
        cloud_angle += (float)(M_PI * 0.4 * dt);
        angle_change += dt;
        angle_triangle_change += dt;
        if (angle_change > 1.5f) {
            angle_change = 0;
            switch (current_axis) {
                case gore::axis::x:
                    current_axis = gore::axis::y;
                break;
                case gore::axis::y:
                    current_axis = gore::axis::z;
                break;
                case gore::axis::z:
                    current_axis = gore::axis::x;
                break;
                case gore::axis::nil:
                break;
            }
        }
        if (angle_triangle_change > 1.5f) {
            angle_triangle_change = 0;
            switch (triangle_axis) {
                case gore::axis::x:
                    triangle_axis = gore::axis::y;
                break;
                case gore::axis::y:
                    triangle_axis = gore::axis::z;
                break;
                case gore::axis::z:
                    triangle_axis = gore::axis::x;
                break;
                case gore::axis::nil:
                break;
            }
        }
        eng.updateInputState();
    }
    return 0;
} 