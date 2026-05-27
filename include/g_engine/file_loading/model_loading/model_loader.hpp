#pragma once
#include <vector>
#include "../../util/vector.hpp"
#include "../../util/matrix.hpp"

namespace gore {
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
    };
    class model {
        private:
            std::vector<model_face> faces;
            matrix model_matrix = matrix(4, 4, 0); // how we project and rotate model into world space            
        public:
            model();
            model (std::vector<model_face> faces);
            // copy
            model (const model& m);
            std::vector<model_face>& getFaces();
            std::vector<gore::vec3> getPositions() const;
    };

    class model_loader {
        public:
        model_loader() = delete;
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        static model loadObj (std::string file_path);
        // https://www.khronos.org/gltf/#gltf-spec
        static std::vector<gore::vec3> loadGltf (std::string file_path);
    };

}