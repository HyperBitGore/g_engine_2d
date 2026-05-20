#pragma once
#include <vector>
#include "../../util/vector.hpp"

namespace gore {
    class model_loader {
        public:
        model_loader() = delete;
        // https://en.wikipedia.org/wiki/Wavefront_.obj_file
        static std::vector<gore::vec3> loadObj (std::string file_path);
        // https://www.khronos.org/gltf/#gltf-spec
        static std::vector<gore::vec3> loadGltf (std::string file_path);
    };

}