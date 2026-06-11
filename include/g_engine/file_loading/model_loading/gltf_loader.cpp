#include "model_loader.hpp"
#include <sstream>
#include <stdexcept>
#include <unordered_map>

struct JSON {
    std::string label;
    std::unordered_map<std::string, std::string> children;
};

// process JSON label
// map is the label we read
JSON readJSONLabel (std::string str, std::string target_label) {
    size_t offset = 0;
    std::string current_label = "";
    for (; offset < str.size();) {
        char c = str[offset];
        switch (c) {
            case '\"':
            {
                // read till end of quote
                size_t i = offset + 1;
                std::string label;
                for (; i < str.size() && str[i] != '\"'; i++) {
                    label.push_back(str[i]);
                }
                current_label = label;
                i++;
                offset = i;
            }
            break;
            case '{':
            {
                // read entire brackets
            }
            break;
            default:
                offset++;
            break;
        }
    }
    return { "NULL", {}};
}


gore::model gore::model_loader::loadGltf (std::string file_path) {
    std::stringstream ss;
    std::ifstream f;
    f.open(file_path);
    ss << f.rdbuf();
    f.close();
    std::string str = ss.str();
    // parse the asset label
    JSON asset = readJSONLabel(str, "asset");
    if (asset.label == "NULL") {
        throw std::runtime_error("Failed to read asset label, invalid GLTF file! " + file_path);
    }
    gore::model m;

    return m;
}