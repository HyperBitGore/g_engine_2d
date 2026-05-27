#pragma once
#include "renderer.hpp"
// https://www.scratchapixel.com/index.html
namespace gore{
    struct threedee_vertex {
        float x;
        float y;
        float z;
        float uvx;
        float uvy;
        GLint model_matrice; // if this negative 1 don't utilize ssbo just render the vertex as is
        GLint texture_unit; // if this negative 1 don't utilize texture sampler, use color uniform
    };
    class threedeerender : public renderer<threedeerender, threedee_vertex> {
        private:
            threedeerender () {
                
            }
            friend class renderer<threedeerender, threedee_vertex>;
	        void shader_setup() override;
            threedeerender(size_t w, size_t h);
        public:
            float vertical_fov = 45.0f;
            float near_clip = 0.1f;
            float far_clip = 100.0f;
            void addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3);
            void addVertexs(const std::vector<gore::vec3>& vertexs);
            void drawBuffer() override;
            // matrices
            void setDimensions (uint32_t width, int32_t height) override;
            void updateView (gore::vec3 camera_pos, gore::vec3 camera_target, gore::vec3 upVector);
    };
}