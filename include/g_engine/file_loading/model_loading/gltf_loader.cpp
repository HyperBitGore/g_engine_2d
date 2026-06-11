#include "model_loader.hpp"
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#define ACCESSOR_SIGNED_BYTE 5120
#define ACCESSOR_UNSIGNED_BYTE 5121
#define ACCESSOR_SIGNED_SHORT 5122
#define ACCESSOR_UNSIGNED_SHORT 5123
#define ACCESSOR_UNSIGNED_INT 5125
#define ACCESSOR_FLOAT 5126

/*
glTF uses a right-handed coordinate system. glTF defines +Y as up, +Z as forward, and -X as right; the front of a glTF asset faces +Z.
*/


struct JSON {
    std::string label;
    std::unordered_map<std::string, std::string> children;
};

// new flow
//  - if label matches
//      - read string block
//      - feed into parseJSONSection
//      -   recursively parse children of section if subsection { or [
//      -   otherwise add child label
//      -   return the parsed JSON
//  - feed up into readJSONLabel
//  - return said JSON

JSON parseJSONSection (std::string section) {

    return { "NULL", {}};
}

// process JSON label
// map is the label we read
JSON readJSONLabel (std::string str, std::string target_label) {
    size_t offset = 0;
    std::string current_label = "";
    for (; offset < str.size();) {
        char c = str[offset];
        switch (c) {
            case '"':
            {
                // read till end of quote
                size_t i = offset + 1;
                std::string label;
                for (; i < str.size() && str[i] != '\"'; i++) {
                    label.push_back(str[i]);
                }
                current_label = label;
                if (current_label == target_label) {
                    // parse the container into a JSON
                    size_t j = i + 1; 
                    // read label until the opening bracket
                    for (;j < str.size() && str[j] != '{'; j++);
                    j++; // skip {
                    if (j < str.size()) {
                        JSON result;
                        result.label = target_label;
                        std::string key;
                        bool expect_key = true;
                        while (j < str.size() && str[j] != '}') {
                            if (isspace((unsigned char)str[j]) || str[j] == ',') { j++; continue; }
                            switch (str[j]) {
                                case '"':
                                {
                                    j++;
                                    std::string s;
                                    for (; j < str.size() && str[j] != '"'; j++) {
                                        if (str[j] == '\\' && j + 1 < str.size()) { j++; s.push_back(str[j]); }
                                        else s.push_back(str[j]);
                                    }
                                    j++; // skip closing '"'
                                    if (expect_key) { key = s; }
                                    else { result.children[key] = s; expect_key = true; }
                                }
                                break;
                                case '{':
                                case '[':
                                // TODO: make this parse recursively so easier to read data we want out of labels
                                    {
                                        char open = str[j], close = str[j] == '{' ? '}' : ']';
                                        size_t start = j++;
                                        int depth = 1;
                                        for (; j < str.size() && depth > 0; j++) {
                                            if (str[j] == open) depth++;
                                            else if (str[j] == close) depth--;
                                        }
                                        result.children[key] = str.substr(start, j - start);
                                        expect_key = true;
                                    }
                                break;
                                case ':':
                                    expect_key = false;
                                    j++;
                                break;
                                default:
                                    {
                                        // scalar: number, bool, null
                                        std::string val;
                                        while (j < str.size() && str[j] != ',' && str[j] != '}') {
                                            if (!isspace((unsigned char)str[j])) val.push_back(str[j]);
                                            j++;
                                        }
                                        result.children[key] = val;
                                        expect_key = true;
                                    }
                                break;
                            }
                        }
                        return result;
                    }
                }
                i++;
                offset = i;
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
    JSON nodes = readJSONLabel(str, "nodes");
    if (nodes.label == "NULL") {
        throw std::runtime_error("Failed to read nodes label!" + file_path);
    }
    JSON buffers = readJSONLabel(str, "buffers");
    JSON bufferViews = readJSONLabel(str, "bufferViews");
    JSON accessors = readJSONLabel(str, "accessors");
    gore::model m;

    return m;
}