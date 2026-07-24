#pragma once
#include <string>
#include <vector>
#include "../../util/vector.hpp"
#include "../../util/matrix.hpp"
#include "../../img_loading/image_loader.hpp"
#include "../json.hpp"
#include "../../rendering/index_buffer.hpp"

namespace gore {
    enum class ModelType { GLTF, OBJ, UNLOADED };
    namespace model_material {
        struct mtl_material {
            std::string name;
            gore::vec3 Ka = {0.2f, 0.2f, 0.2f}; // ambient
            gore::vec3 Kd = {0.8f, 0.8f, 0.8f}; // diffuse
            gore::vec3 Ks = {0.0f, 0.0f, 0.0f}; // specular
            gore::vec3 Ke = {0.0f, 0.0f, 0.0f}; // emissive
            float Ns  = 10.0f; // specular exponent
            float d   = 1.0f;  // dissolve (opacity)
            float Ni  = 1.0f;  // index of refraction
            int illum = 2;     // illumination model
            std::string map_Kd; // diffuse texture path
            std::string map_Ka; // ambient texture path
            std::string map_Ks; // specular texture path
            std::string map_bump; // bump map path;
            std::string map_d; // transparency map
        };

        enum class AlphaMode { ALPHA_OPAQUE, ALPHA_MASK, ALPHA_BLEND };
        struct gltf_material {
            std::string name;
            // PBR metallic-roughness
            gore::vec4 base_color_factor      = {1.0f, 1.0f, 1.0f, 1.0f};
            float      metallic_factor        = 1.0f;
            float      roughness_factor       = 1.0f;
            int tex_base_color        = -1;
            int tex_metallic_roughness= -1;
            // normal texture
            int tex_normal            = -1;
            float normal_scale = 1.0f;
            // occlusion texture
            int tex_occlusion         = -1;
            float occlusion_strength = 1.0f;
            // emissive
            int tex_emissive          = -1;
            gore::vec3 emissive_factor        = {0.0f, 0.0f, 0.0f};
            // alpha
            AlphaMode alpha_mode      = AlphaMode::ALPHA_OPAQUE;
            float     alpha_cutoff    = 0.5f; // only used when alpha_mode == ALPHA_MASK
            bool      double_sided    = false;
        };
    }

    enum class WindingOrder { CCW, CW };

    struct model_face {
        gore::vec3 p1;
        gore::vec3 p2;
        gore::vec3 p3;
        gore::vec3 norm1;
        gore::vec3 norm2;
        gore::vec3 norm3;
        gore::vec2 uv1;
        gore::vec2 uv2;
        gore::vec2 uv3;
        int material_index = -1;
        WindingOrder winding_order = WindingOrder::CCW;
    };
    struct model_vertex {
        gore::vec3 pos;
        gore::vec3 norm;
        gore::vec2 uv;
        int material_index = -1;

        bool operator==(const model_vertex& v) const {
            return pos.x == v.pos.x && pos.y == v.pos.y && pos.z == v.pos.z
                && norm.x == v.norm.x && norm.y == v.norm.y && norm.z == v.norm.z
                && uv.x == v.uv.x && uv.y == v.uv.y
                && material_index == v.material_index;
        }
    };
}

template <>
struct std::hash<gore::model_vertex> {
    size_t operator()(const gore::model_vertex& v) const noexcept {
        size_t seed = std::hash<int>{}(v.material_index);
        auto combine = [&seed](float f) {
            seed ^= std::hash<float>{}(f) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        combine(v.pos.x); combine(v.pos.y); combine(v.pos.z);
        combine(v.norm.x); combine(v.norm.y); combine(v.norm.z);
        combine(v.uv.x); combine(v.uv.y);
        return seed;
    }
};

namespace gore {
    // treat this kind've like a texture you load and use its contents to create objects you actually feed into render pipeline
    class model {
        private:
            size_t texture_count = 0;
            std::vector<model_material::mtl_material> mtls;
            std::vector<model_material::gltf_material> gltfs;
            static int hash(std::string str) {
		        return (str.size() > 0) ? str[0] + str[str.size() - 1] % 1024 : 0;
	        }
            std::vector<IMG> images;
            matrix model_matrix = matrix(4, 4, 0); // how we project and rotate model into world space
            model_material::mtl_material* getMTLMat (int32_t index);
            model_material::gltf_material* getGLTFMat (int32_t index);
        public:
            ModelType type;
            hashmap<uint32_t, std::string> image_map;
            index_buffer<model_vertex> index_buffer;
            model();
            ~model();
            model (gore::index_buffer<model_vertex> buffer, const ModelType type);
            // copy
            model (const model& m);
            // move
            model (model&& m) noexcept;
            // copy assignment
            model& operator=(const model& m);
            // move assignment
            model& operator=(model&& m) noexcept;
            
            std::vector<gore::vec3> getPositions() const;
            void addMaterials (const std::vector<model_material::mtl_material>& mats);
            void addMaterials (std::vector<model_material::gltf_material>& mats);
            void addMaterials (std::vector<model_material::gltf_material>& mats, std::vector<IMG>& imgs);
            IMG& getImage (int32_t material_index);
            // Add a pre-loaded image as a new material, avoiding file I/O.
            // The face's material_index must equal the mtls index this creates.
            void addImageMaterialMTL(IMG img, const std::string& key);
            void translate (vec3 translation);
            void rotate (vec3 axis, float radians);
            void scale (vec3 scale);
            void resetMatrix ();
            matrix& getMatrix () {
                return model_matrix;
            }
            size_t textureCount () {
                return texture_count;
            }
    };

    class model_loader {
        public:
        model_loader() = delete;
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        static model loadObj (std::string file_path);
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library
        static std::vector<model_material::mtl_material> loadMtl (std::string file_path);
        // https://www.khronos.org/gltf/#gltf-spec
        static model loadGltf (std::string file_path);
    };
}