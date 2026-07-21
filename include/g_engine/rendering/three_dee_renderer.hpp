#pragma once
#include "renderer.hpp"
#include "billboard.hpp"
#include "../file_loading/model_loading/model_loader.hpp"
#include <unordered_map>
// https://www.scratchapixel.com/index.html
// TODO
//      - element buffer actually static instead of reallocating every frame
//      - texture unit setting with new static element buffer
//      - model matrices static
namespace gore{
    struct threedee_vertex {
        float x;
        float y;
        float z;
        float uvx;
        float uvy;
        GLint model_matrice;
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
            GLuint element_buffer;
            std::vector<GLuint> indexs;
            // per-call cached geometry (triangles/billboards), appended after persistent model geometry
            std::vector<threedee_vertex> transient_vertexs;
            struct transient_entry {
                std::vector<threedee_vertex> verts;
                size_t offset;           // offset into transient_vertexs
                uint64_t last_frame;     // residency stamp
            };
            std::unordered_map<size_t, transient_entry> transient_map;
            static size_t hashVertexs (const threedee_vertex* data, size_t count);
            void addTransient (const threedee_vertex* data, size_t count);
            void rebuildTransients ();
            std::vector<GLuint> bound_textures;
            void rebuildGeometry ();
            void uploadMatrices ();
            GLuint index_allocated = 0;
            // instances
            struct instance {
                size_t index_offset, index_count;
                size_t vertex_offset, vertex_count;
                GLint mat_slot;          // stable slot while resident
                uint64_t last_frame;     // residency stamp
            };
            std::unordered_map<model*, instance> model_map;
            uint64_t frame_count = 0;
            bool buffers_dirty = false;
	        void shader_setup() override;
            threedeerender(size_t w, size_t h);
            void updateVertexBuffer ();
        public:
            float vertical_fov = 45.0f;
            float near_clip = 0.1f;
            float far_clip = 100.0f;
            void addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3);
            void addVertexs(const std::vector<gore::vec3>& vertexs);
            void addModel (gore::model& model);
            void addBillboard (gore::billboard& billboard, gore::camera& cam);
            void addModelInstance (gore::model* model, const matrix& transform);
            void drawBuffer() override;
            // matrices
            void updateDimensions (uint32_t width, uint32_t height) override;
    };
}