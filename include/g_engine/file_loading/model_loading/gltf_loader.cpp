#include "model_loader.hpp"
#include <cctype>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
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
enum class JSONTYPE { STRING, OBJECT, ARRAY };

struct Element {
    JSONTYPE type;
    Element(JSONTYPE t) : type(t) {}
    virtual ~Element() = default;
};

// either a string or JSON struct
template <class T>
struct JSONElement : public Element {
    T value;
     JSONElement(JSONTYPE t, T v) : Element{t}, value(std::move(v)) {}
};

struct JSON {
    std::string label;
    std::unordered_map<std::string, std::unique_ptr<Element>> children;
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

// TODO: FINISH THIS ->
// process arrays with values and no labels
// properly process bufferViews
// nodes

std::string readBrackets (std::string section, size_t* offset) {
    size_t i = *offset + 1; // assuming we starting on the bracket
    std::string new_section;
    int depth = 1;
    for (; i < section.size() && depth > 0; i++) {
        switch (section[i]) {
            case '{':
            depth++;
            break;
            case '}':
            depth--;
            break;
        }
        if (depth > 0) {
            new_section.push_back(section[i]);
        }
    }
    i++;
    *offset = i;
    return new_section;
}
JSON parseJSONSection (std::string section);

std::vector<std::unique_ptr<Element>> readArray (std::string section, size_t* offset) {
    size_t i = *offset + 1;
    int depth = 1;
    std::string thing;
    std::vector<std::unique_ptr<Element>> items;
    for (; i < section.size() && depth > 0; ) {
        switch (section[i]) {
            case '[':
                // should probably parse this in recursive way
                depth++;
                i++;
            break;
            case ']':
                depth--;
                i++;
            break;
            case '{':
                {
                    // process the JSON object
                    std::string new_section = readBrackets(section, &i);
                    JSON child = parseJSONSection(new_section);
                    std::unique_ptr<JSONElement<JSON>> element = std::make_unique<JSONElement<JSON>>(JSONTYPE::OBJECT, std::move(child)); 
                    items.push_back(std::move(element));
                }
            break;
            case ',':
            case '\n':
                {
                    std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, thing); 
                    items.push_back(std::move(element));
                    thing.clear();
                    i++;
                }
            break;
            default:
                {
                    // combine until newline or ,
                    thing.push_back(section[i]);
                    i++;
                }
            break;
        }
    }                    
    *offset = i;
    return items;
}

JSON parseJSONSection (std::string section) {
    size_t offset = 0;
    JSON output = {"NULL", {}};
    size_t free = 0;
    for (; offset < section.size();) {
        switch (section[offset]) {
            case '"':
                {
                    size_t i = offset + 1;
                    std::string label;
                    for (; i < section.size() && section[i] != '"'; i++) {
                        label.push_back(section[i]);
                    }
                    i++;
                    // now read the data ahead of it
                    // read until we hit a non white or :
                    for (; i < section.size() && (std::isspace(section[i]) || section[i] == ':'); i++);
                    if (section[i] == '{') {
                        // recursive json parse
                        std::string new_section = readBrackets(section, &i);
                        JSON child = parseJSONSection(new_section);
                        child.label = label;
                        std::unique_ptr<JSONElement<JSON>> element = std::make_unique<JSONElement<JSON>>(JSONTYPE::OBJECT, std::move(child));
                        output.children.emplace(label, std::move(element));
                    } else if (section[i] == '[') {
                        std::vector<std::unique_ptr<Element>> elements = readArray(section, &i);
                        std::unique_ptr<JSONElement<std::vector<std::unique_ptr<Element>>>> element = std::make_unique<JSONElement<std::vector<std::unique_ptr<Element>>>>(JSONTYPE::ARRAY, std::move(elements));
                        output.children.emplace(label, std::move(element));
                    } else {
                        // read until newline or ,
                        std::string value;
                        for (; i < section.size() && (section[i] != ',') && section[i] != '\n'; i++) {
                            value.push_back(section[i]);
                        }
                        i++;
                        std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, value);
                        output.children.emplace(label, std::move(element));
                    }
                    offset = i;

                }
            break;
            case '[':
                {
                    std::vector<std::unique_ptr<Element>> elements = readArray(section, &offset);
                }
            break;
            case '{':
                {
                    std::string new_section = readBrackets(section, &offset);
                    JSON child = parseJSONSection(new_section);
                    return child;
                }
            default:
                offset++;
        }
    }
    return output;
}

// process JSON label

JSON processJSONFile (std::string str) {
    return parseJSONSection(str);
}

JSON* getLabelJSON (JSON* json, std::string label) {
    auto& it = json->children[label];
    JSONElement<JSON>* elem = dynamic_cast<JSONElement<JSON>*>(it.get());
    if (!elem) return nullptr;
    return &elem->value;
}

std::vector<std::unique_ptr<Element>>* getLabelArray (JSON* json, std::string label) {
    auto& it = json->children[label];
    JSONElement<std::vector<std::unique_ptr<Element>>>* elem = dynamic_cast<JSONElement<std::vector<std::unique_ptr<Element>>>*>(it.get());
    if (!elem) return nullptr;
    return &elem->value;
}


gore::model gore::model_loader::loadGltf (std::string file_path) {
    std::stringstream ss;
    std::ifstream f;
    f.open(file_path);
    ss << f.rdbuf();
    f.close();
    std::string str = ss.str();
    JSON processed = processJSONFile(str);
    // parse the asset label
    JSON* asset = getLabelJSON(&processed, "asset");
    if (!asset) {
        throw std::runtime_error("Failed to read asset label, invalid GLTF file! " + file_path);
    }
    std::vector<std::unique_ptr<Element>>* nodes = getLabelArray(&processed, "nodes");
    std::vector<std::unique_ptr<Element>>* buffers = getLabelArray(&processed, "buffers");
    std::vector<std::unique_ptr<Element>>* bufferViews = getLabelArray(&processed, "bufferViews");
    std::vector<std::unique_ptr<Element>>* accessors = getLabelArray(&processed, "accessors");
    std::vector<std::unique_ptr<Element>>* cameras = getLabelArray(&processed, "cameras");
    gore::model m;

    return m;
}