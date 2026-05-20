#pragma once
#include "image_renderer.hpp"
// https://www.scratchapixel.com/index.html
namespace gore{
    class threedeerenderer {
        private:
            struct ivertex {
                float x;
                float y;
                float z;
                float uvx;
		        float uvy;
                GLint texture_unit; // if this negative 1 don't utilize texture sampler, use color uniform
            };
            std::vector<ivertex> buffer;
            shader shader;
            GLuint vao;
            GLuint vertex_buffer;
            GLuint allocated;
            uint32_t width, height;
            int32_t texture_units;
            threedeerenderer () {

            }
        public:
            threedeerenderer(size_t w, size_t h);
            // copy
            threedeerenderer(const threedeerenderer& img);
            void addVertex(gore::vec3 pos);
            void addVertexs(const std::vector<gore::vec3>& vertexs);
            void drawBuffer();

    };
}