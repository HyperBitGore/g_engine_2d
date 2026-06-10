#pragma once
#include "renderer.hpp"
#include "../file_loading/model_loading/model_loader.hpp"
// https://www.scratchapixel.com/index.html
namespace gore{
    struct threedee_vertex {
        float x;
        float y;
        float z;
        float uvx;
        float uvy;
        GLuint model_matrice;
        GLuint texture_unit;
    };
    class threedeerender : public renderer<threedeerender, threedee_vertex> {
        private:
            threedeerender () {
                texture_unit_map.setHashFunction(hash);
            }
            friend class renderer<threedeerender, threedee_vertex>;
            uint32_t current_unit = 0;
            static int hash(GLuint texture) {
                return texture % 512;
            }
            gore::hashmap<GLuint, GLuint> texture_unit_map;
            std::vector<GLint> samplers;
            GLuint getTextureUnit (GLuint texture);
            bool textureBinded (GLuint texture);
            void setTextureSamplers ();
            // model matrice count
            std::vector<matrix> model_matrices;
            GLuint ssbo;
            matrix identity = matrix::generateIdentity(4, 4);
	        void shader_setup() override;
            threedeerender(size_t w, size_t h);
        public:
            float vertical_fov = 45.0f;
            float near_clip = 0.1f;
            float far_clip = 100.0f;
            void addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3);
            void addVertexs(const std::vector<gore::vec3>& vertexs);
            void addModel (gore::model& model);
            void drawBuffer() override;
            // matrices
            void updateDimensions (uint32_t width, uint32_t height) override;
    };
}