#pragma once
#include "renderer.hpp"

namespace gore{
    enum class axis { x = 0, y = 1, z = 2, nil = 3};
    struct wireframe_vertex {
        float x;
        float y;
        float z;
        float rotation;
        float rotation_x;
        float rotation_y;
        float rotation_z;
        GLuint axis;
    };
    class wireframe_renderer : public renderer<wireframe_renderer, wireframe_vertex> {
        private:
            friend class renderer<wireframe_renderer, wireframe_vertex>;
            void shader_setup () override;
            wireframe_renderer (uint32_t width, uint32_t height);
        public:
            void addVertex(gore::vec3 p, float rotation, axis axis, gore::vec3 rotation_point);
            void addVertex (gore::vec3 p);
            void addLine (gore::vec3 p1, gore::vec3 p2, float rotation, axis axis, gore::vec3 rotation_point);
            void addLine (gore::vec3 p1, gore::vec3 p2);
            void addTriangle (gore::vec3 p1, gore::vec3 p2, gore::vec3 p3, float rotation, axis axis, gore::vec3 rotation_point);
            void addTriangle (gore::vec3 p1, gore::vec3 p2, gore::vec3 p3);
            void drawBuffer() override;
            void setColor (gore::vec4 color);
    };

}