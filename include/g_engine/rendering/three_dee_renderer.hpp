#pragma once
#include "renderer.hpp"
#include "billboard.hpp"
#include "../file_loading/model_loading/model_loader.hpp"
#include <unordered_map>
// https://www.scratchapixel.com/index.html
// TODO
//      - glMultiDrawElementsIndirect
//      - first matrix always an identity matrix (billboard and transients target this, so first drawcommand should be transients)
//      - write seperate glMultiDraw elementsIndirect, then convert three renderer from that
namespace gore{
    PREVENT_PACKING_STRUCT  DrawElementsIndirectCommand {
        GLuint count;
        GLuint instance_count;
        GLuint first_index;
        GLint base_vertex;
        GLuint base_instance;
    };
    END_PACKING_STRUCT

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
            enum class KEY_TYPE { MODEL, TRIANGLE, BILLBOARD };
            // per-call cached geometry (triangles/billboards), appended after persistent model geometry
            std::vector<threedee_vertex> transient_vertexs;
            void addTransient (const threedee_vertex* data, size_t count);
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
            uint64_t frame_count = 0;
            bool buffers_dirty = false;
            struct draw_key {
                size_t value;
                KEY_TYPE type;
                bool operator==(const draw_key& other) const {
                    return value == other.value;
                }
            };
            struct draw_key_hash {
                size_t operator()(const draw_key& key) const {
                    return std::hash<size_t>{}(key.value);
                }
            };
            std::unordered_map<draw_key, instance, draw_key_hash> draw_map;
            // instancing
            std::vector<DrawElementsIndirectCommand> draw_commands;
            std::unordered_map<model*, size_t> draw_call_map;
            void addDrawCall (gore::model* m, matrix matrix);
            void addModelData (gore::model* m);
            GLuint indirect_buffer = 0;
	        void shader_setup() override;
            threedeerender(size_t w, size_t h);
            void updateVertexBuffer ();
            instance& getModelInstance (model* m) {
                const draw_key key{reinterpret_cast<size_t>(m), KEY_TYPE::MODEL};
                auto it = draw_map.find(key);
                return it->second;
            }
        public:
            float vertical_fov = 45.0f;
            float near_clip = 0.1f;
            float far_clip = 100.0f;
            void addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3);
            void addModel (gore::model& model);
            void addBillboard (gore::billboard& billboard, gore::camera& cam);
            void addModelInstance (gore::model* model, const matrix& transform);
            void drawBuffer() override;
            // matrices
            void updateDimensions (uint32_t width, uint32_t height) override;
    };
    struct instance_vertex {
        float x;
        float y;
        float z;
        float uvx;
        float uvy;
        GLuint texture_unit;
    };
    // new flow
    //  - addModelData
    //      - adds model data and you preallocate the amt of instances you need?
    //  - addModelInstance
    //      - adds model instance in open space, and updates matrix
    //      - use returned index to modify the matrix
    //  - removeModelInstance
    //      - removes instance index
    //      - copies ones ahead of it back
    class instance_render : public renderer<instance_render, instance_vertex> {
        private:
            friend class renderer<instance_render, instance_vertex>;
            std::vector<GLuint> indexs;
            std::vector<DrawElementsIndirectCommand> commands;
            std::vector<matrix> model_matrices;
            GLuint ssbo;
            GLuint element_buffer;
            GLuint draw_buffer;
            struct instance {
                int32_t command; // index of command
                size_t index_offset, index_count;
                size_t vertex_offset, vertex_count;
            };
            std::unordered_map<model*, instance> instance_map;
            void shader_setup() override;
            instance_render(size_t w, size_t h);
            bool buffers_dirty = false;
            bool draw_buffer_dirty = false;
            void updateDrawBuffers ();
        public:
            float vertical_fov = 45.0f;
            float near_clip = 0.1f;
            float far_clip = 100.0f;
            int32_t addModelInstance (gore::model* model, const matrix& transform);
            void addModelData(gore::model* model, size_t preallocate);
            void removeModelInstance (gore::model* model, int32_t index);
            // matrices
            void updateDimensions (uint32_t width, uint32_t height) override;
            void drawBuffer() override;
    };
}