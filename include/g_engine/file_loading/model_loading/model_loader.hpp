#pragma once
#include <string>
#include <vector>
#include "../../util/vector.hpp"
#include "../../util/matrix.hpp"
#include "../../img_loading/image_loader.hpp"

namespace gore {
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
            // emissive
            gore::vec3 emissive_factor        = {0.0f, 0.0f, 0.0f};
            // texture indices into the gltf textures array (-1 = not set)
            int tex_base_color        = -1;
            int tex_metallic_roughness= -1;
            int tex_normal            = -1;
            int tex_occlusion         = -1;
            int tex_emissive          = -1;
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
    // treat this kind've like a texture you load and use its contents to create objects you actually feed into render pipeline
    class model {
        private:
            std::vector<model_face> faces;
            std::vector<model_material::mtl_material> mtls;
            std::vector<model_material::gltf_material> gltfs;
            static int hash(std::string str) {
		        return (str.size() > 0) ? str[0] + str[str.size() - 1] % 1024 : 0;
	        }
            std::vector<IMG> images;
            matrix model_matrix = matrix(4, 4, 0); // how we project and rotate model into world space
            model_material::mtl_material* getMTLMat (int32_t index);   
        public:
            hashmap<uint32_t, std::string> image_map;
            model();
            ~model();
            model (std::vector<model_face> faces);
            // copy
            model (const model& m);
            // move
            model (model&& m) noexcept;
            // copy assignment
            model& operator=(const model& m);
            // move assignment
            model& operator=(model&& m) noexcept;
            
            std::vector<model_face>& getFaces();
            std::vector<gore::vec3> getPositions() const;
            void addMaterials (const std::vector<model_material::mtl_material>& mats);
            void addMaterials (const std::vector<model_material::gltf_material>& mats);
            IMG& getImage (int32_t mtl_index);
            // Add a pre-loaded image as a new material, avoiding file I/O.
            // The face's material_index must equal the mtls index this creates.
            void addImageMaterial(IMG img, const std::string& key);
            void translate (vec3 translation);
            void rotate (vec3 axis, float radians);
            void resetMatrix ();
            matrix getMatrix () {
                return model_matrix;
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