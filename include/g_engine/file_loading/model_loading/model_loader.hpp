#pragma once
#include <vector>
#include "../../util/vector.hpp"
#include "../../util/matrix.hpp"

namespace gore {

    class model {
        private:
            gore::vec3 position;
            std::vector<vec3> vertexs;
            matrix model_matrix = matrix(4, 4, 0); // how we project and rotate model into world space            
        public:
            model() = delete;
            model (std::vector<vec3> vertexs);
            // copy
            model (const model& m);
    };

    class model_loader {
        public:
        model_loader() = delete;
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        static std::vector<gore::vec3> loadObj (std::string file_path);
        // https://www.khronos.org/gltf/#gltf-spec
        static std::vector<gore::vec3> loadGltf (std::string file_path);
    };

}