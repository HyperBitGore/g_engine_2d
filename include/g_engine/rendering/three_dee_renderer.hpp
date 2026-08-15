#pragma once
#include "renderer.hpp"
#include "billboard.hpp"
#include "../file_loading/model_loading/model_loader.hpp"
#include "texture_unit_manager.hpp"
#include "../util/partition_array.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace gore {
    PREVENT_PACKING_STRUCT DrawElementsIndirectCommand {
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
        GLuint texture_index;
    };

    class threedeerender : public renderer<threedeerender, threedee_vertex> {
    protected:
        friend class renderer<threedeerender, threedee_vertex>;
        threedeerender(std::string vertex_shader, std::string fragment_shader,
                       uint32_t width, uint32_t height)
            : renderer<threedeerender, threedee_vertex>(
                std::move(vertex_shader), std::move(fragment_shader), width, height) {}

        virtual GLuint textureIndex(GLuint texture) = 0;
        virtual void setTextureSamplers() = 0;
        virtual void clearTextures() = 0;

        GLuint texture_ssbo = 0;
        std::vector<matrix> model_matrices;
        GLuint ssbo = 0;
        GLuint element_buffer = 0;
        std::vector<GLuint> indexs;
        enum class KEY_TYPE { MODEL, TRIANGLE, BILLBOARD };
        std::vector<threedee_vertex> transient_vertexs;
        void addTransient(const threedee_vertex* data, uint32_t count);
        std::vector<GLuint> bound_textures;
        void rebuildGeometry();
        void flushTextureBatch();
        void uploadMatrices();
        GLuint index_allocated = 0;
        struct instance {
            uint32_t index_offset, index_count;
            uint32_t vertex_offset, vertex_count;
            GLint mat_slot;
            uint64_t last_frame;
        };
        uint64_t frame_count = 0;
        bool buffers_dirty = false;
        struct draw_key {
            uintptr_t value;
            KEY_TYPE type;
            bool operator==(const draw_key& other) const {
                return value == other.value;
            }
        };
        struct draw_key_hash {
            uintptr_t operator()(const draw_key& key) const {
                return std::hash<uintptr_t>{}(key.value);
            }
        };
        std::unordered_map<draw_key, instance, draw_key_hash> draw_map;

        void updateVertexBuffer();
        void shader_setup() override = 0;

    public:
        using renderer<threedeerender, threedee_vertex>::create;
        virtual ~threedeerender() = default;
        float vertical_fov = 45.0f;
        float near_clip = 0.1f;
        float far_clip = 100.0f;
        void addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3);
        void addModel(gore::model& model);
        void addBillboard(gore::billboard& billboard, gore::camera& cam);
        void drawBuffer() override;
        void updateDimensions(uint32_t width, uint32_t height) override;
        static std::unique_ptr<threedeerender> create(uint32_t width, uint32_t height);
    };

    class bindless_threedeerender final : public threedeerender {
    protected:
        friend class renderer<threedeerender, threedee_vertex>;
        bindless_texture_manager tm;
        bindless_threedeerender(uint32_t width, uint32_t height);
        GLuint textureIndex(GLuint texture) override;
        void setTextureSamplers() override;
        void clearTextures() override;
        void shader_setup() override;
    public:
    };

    class texture_unit_threedeerender final : public threedeerender {
    protected:
        friend class renderer<threedeerender, threedee_vertex>;
        texture_unit_manager tm;
        texture_unit_threedeerender(uint32_t width, uint32_t height);
        GLuint textureIndex(GLuint texture) override;
        void setTextureSamplers() override;
        void clearTextures() override;
        void shader_setup() override;
    public:
    };

    struct instance_vertex {
        float x;
        float y;
        float z;
        float uvx;
        float uvy;
    };

    class instance_render : public renderer<instance_render, instance_vertex> {
    private:
        friend class renderer<instance_render, instance_vertex>;
        std::vector<GLuint> indexs;
        std::vector<DrawElementsIndirectCommand> commands;
        std::vector<float> matrix_array;
        uint32_t current_matrix_size = 0;
        void preallocateMatrixArray(model* model);
        void reallocateMatrixArray(model* model);
        int32_t addModelMatrix(model* model, const matrix& transform);
        int32_t removeModelMatrix(model* model, int32_t index);
        void setModelMatrix(model* model, int32_t index);
        GLuint ssbo;
        GLuint element_buffer;
        GLuint draw_buffer;
        struct instance {
            int32_t command;
            uint32_t index_offset, index_count;
            uint32_t vertex_offset, vertex_count;
            uint32_t matrix_offset;
            uint32_t matrix_size;
            uint32_t current_matrix_index = 0;
            uint32_t tex_unit;
        };
        std::vector<instance> instance_array;
        std::unordered_map<model*, uint32_t> instance_map;
        bindless_texture_manager tm;
        partition_array texture_partition_array;
        GLuint texure_ssbo;
        GLuint texture_index_buffer;
        bool texture_partition_dirty = false;
        void shader_setup() override;
        instance_render(uint32_t width, uint32_t height);
        bool buffers_dirty = false;
        bool draw_buffer_dirty = false;
        bool matrix_buffer_dirty = false;
        bool instance_texture_units_dirty = false;
        void updateDrawBuffers();
    public:
        float vertical_fov = 45.0f;
        float near_clip = 0.1f;
        float far_clip = 100.0f;
        int32_t addModelInstance(gore::model* model, const matrix& transform);
        void addModelData(model* model, uint32_t preallocate);
        void removeModelInstance(model* model, int32_t index);
        void updateModelInstance(model* model, int32_t index, const matrix& transform);
        void updateDimensions(uint32_t width, uint32_t height) override;
        void drawBuffer() override;
    };

    extern std::unique_ptr<threedeerender> createThreeDeeRenderer(uint32_t width, uint32_t height);
}
