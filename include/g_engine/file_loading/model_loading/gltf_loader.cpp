#include "model_loader.hpp"
#include <cctype>
#include <filesystem>
#include <fstream>
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
#define ARRAY_BUFFER 34962
#define ELEMENT_ARRAY_BUFFER 34963

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

std::string trimQuotesAndWhitespace(const std::string& str) {
    size_t start = 0, end = str.size();
    
    // Trim leading whitespace
    while (start < end && std::isspace(str[start])) start++;
    // Trim trailing whitespace
    while (end > start && std::isspace(str[end - 1])) end--;
    
    // Trim quotes
    if (start < end && str[start] == '"') start++;
    if (end > start && str[end - 1] == '"') end--;
    
    return str.substr(start, end - start);
}

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
                    if (!thing.empty()) {
                        std::string trimmed = trimQuotesAndWhitespace(thing);
                        if (!trimmed.empty()) {
                            std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, trimmed); 
                            items.push_back(std::move(element));
                        }
                    }
                    thing.clear();
                    i++;
                }
            break;
            default:
                {
                    // combine until newline or ,
                    if (!std::isspace(section[i])) {
                        thing.push_back(section[i]);
                    }
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
                        std::string trimmed = trimQuotesAndWhitespace(value);
                        if (!trimmed.empty()) {
                            std::unique_ptr<JSONElement<std::string>> element = std::make_unique<JSONElement<std::string>>(JSONTYPE::STRING, trimmed);
                            output.children.emplace(label, std::move(element));
                        }
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

std::string* getLabelString (JSON* json, std::string label) {
    auto& it = json->children[label];
    JSONElement<std::string>* elem = dynamic_cast<JSONElement<std::string>*>(it.get());
    if (!elem) return nullptr;
    return &elem->value;
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

template <class T>
JSONElement<T>* getElement (std::unique_ptr<Element>& element) {
    return dynamic_cast<JSONElement<T>*>(element.get());
}

std::string JSONToString (JSON* json);

std::string elementArrayToString (std::vector<std::unique_ptr<Element>>* elements) {
    std::string out;
    out.append(": [\n");
    for (const auto& arr_elem : *elements) {
        switch (arr_elem->type) {
            case JSONTYPE::STRING:
                {
                    auto str_elem = dynamic_cast<JSONElement<std::string>*>(arr_elem.get());
                    if (str_elem) out.append("  \"" + str_elem->value + "\",\n");
                }
                break;
            case JSONTYPE::OBJECT:
                {
                    auto obj_elem = dynamic_cast<JSONElement<JSON>*>(arr_elem.get());
                    if (obj_elem) out.append("  " + JSONToString(&obj_elem->value) + ",\n");
                }
                break;
            case JSONTYPE::ARRAY:
                auto array_elem = dynamic_cast<std::vector<std::unique_ptr<Element>>*>(arr_elem.get());
                out.append(elementArrayToString(array_elem));
                break;
        }
    }
    out.append("]");
    
    return out;
}

std::string JSONToString (JSON* json) {
    std::string out = json->label + ": {\n";
    for (auto& i : json->children) {
        if (i.first == "NULL") {
            out.push_back(' ');
        } else {
            out.append(i.first);
        }
        switch (i.second->type) {
            case JSONTYPE::STRING:
                {
                    auto elem = getElement<std::string>(i.second);
                    if (elem) out.append(": " + elem->value + "\n");
                }
            break;
            case JSONTYPE::OBJECT:
                {
                    auto elem = getElement<JSON>(i.second);
                    if (elem) out.append(": " + JSONToString(&elem->value) + "\n");
                }
                break;
            case JSONTYPE::ARRAY:
                {
                    auto elem = getElement<std::vector<std::unique_ptr<Element>>>(i.second);
                    if (elem) {
                        out.append(elementArrayToString(&(elem->value)) + "\n");
                    }
                }
              break;
            }
    }

    out.push_back('}');
    return out;
}

std::vector<std::vector<uint8_t>> readBuffers (std::vector<std::unique_ptr<Element>>* buffers, std::filesystem::path path) {
    // loop through buffers and read
    std::vector<std::vector<uint8_t>> out;
    std::unordered_map<std::string, std::vector<uint8_t>> fileMap;
    for (auto& i : *buffers) {
        if (i->type == JSONTYPE::OBJECT) {
            auto elem = getElement<JSON>(i);
            std::string* length = getLabelString(&(elem->value), "byteLength");
            std::string* uri = getLabelString(&(elem->value), "uri");
            std::vector<uint8_t> p;
            // now read buffer file
            try {
                p = fileMap.at(*uri);
            } catch (std::out_of_range e) {
                // ignore and load file
                std::ifstream file(path / std::filesystem::path(*uri), std::ios::binary);
                if (!file) {
                    throw std::runtime_error("Malformed uri in gltf file!");
                }
                file.seekg(0, std::ios::end);
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<uint8_t> read(size);
                file.read(reinterpret_cast<char*>(read.data()), size);
                file.close();
                fileMap.emplace(*uri, read);
                p = read;
            }
            size_t val = std::stoi(*length);
            std::vector<uint8_t> new_file;
            for (size_t off = 0; off < val; off++) {
                new_file.push_back(p[off]);
            }
            out.push_back(new_file);
        } else {
            throw std::runtime_error("Malformed buffer in gltf file!");
        }
    }
    return out;
}
struct buffer_view {
    int buffer;
    int byteOffset = 0;
    int byteLength;
    int byteStride = -1;
    int target = ARRAY_BUFFER;
    std::string name;
};

std::vector<buffer_view> parseBufferViews (std::vector<std::unique_ptr<Element>>* views) {
    std::vector<buffer_view> out;
    for (auto& i : *views) {
        auto obj_elem = dynamic_cast<JSONElement<JSON>*>(i.get());
        if (!obj_elem) continue;
        
        buffer_view view;
        JSON* obj = &obj_elem->value;
        
        // Parse required fields
        auto buffer_elem = getElement<std::string>(obj->children["buffer"]);
        if (buffer_elem) view.buffer = std::stoi(buffer_elem->value);
        
        auto byteLength_elem = getElement<std::string>(obj->children["byteLength"]);
        if (byteLength_elem) view.byteLength = std::stoi(byteLength_elem->value);
        
        // Parse optional fields
        auto byteOffset_elem = getElement<std::string>(obj->children["byteOffset"]);
        if (byteOffset_elem) view.byteOffset = std::stoi(byteOffset_elem->value);
        
        auto byteStride_elem = getElement<std::string>(obj->children["byteStride"]);
        if (byteStride_elem) view.byteStride = std::stoi(byteStride_elem->value);
        
        auto target_elem = getElement<std::string>(obj->children["target"]);
        if (target_elem) view.target = std::stoi(target_elem->value);
        
        auto name_elem = getElement<std::string>(obj->children["name"]);
        if (name_elem) view.name = name_elem->value;
        
        out.push_back(view);
    }
    return out;
}

struct sparse_indices {
    int bufferView;
    int byteOffset = 0;
    int componentType;
};

struct sparse_values {
    int bufferView;
    int byteOffset = 0;
};

struct sparse {
    int count;
    sparse_indices indices;
    sparse_values values;
};

struct accessor {
    int bufferView;
    int bufferOffset = 0;
    int componentType;
    bool normalized = false;
    int count;
    std::string type;
    std::vector<uint32_t> max; // just convert values to float if needed
    std::vector<uint32_t> min;
    sparse sparse;
    std::string name;
};

std::vector<accessor> parseAccessors (std::vector<std::unique_ptr<Element>>* elements) {
    std::vector<accessor> accessors;
    for (auto& i : *elements) {
        auto obj_elem = dynamic_cast<JSONElement<JSON>*>(i.get());
        if (!obj_elem) continue;
        
        accessor acc;
        JSON* obj = &obj_elem->value;
        
        // Parse required fields
        auto bufferView_elem = getElement<std::string>(obj->children["bufferView"]);
        if (bufferView_elem) acc.bufferView = std::stoi(bufferView_elem->value);
        
        auto componentType_elem = getElement<std::string>(obj->children["componentType"]);
        if (componentType_elem) acc.componentType = std::stoi(componentType_elem->value);
        
        auto count_elem = getElement<std::string>(obj->children["count"]);
        if (count_elem) acc.count = std::stoi(count_elem->value);
        
        auto type_elem = getElement<std::string>(obj->children["type"]);
        if (type_elem) acc.type = type_elem->value;
        
        // Parse optional fields
        auto byteOffset_elem = getElement<std::string>(obj->children["byteOffset"]);
        if (byteOffset_elem) acc.bufferOffset = std::stoi(byteOffset_elem->value);
        
        auto normalized_elem = getElement<std::string>(obj->children["normalized"]);
        if (normalized_elem) acc.normalized = (normalized_elem->value == "true");
        
        auto max_elem = getElement<std::vector<std::unique_ptr<Element>>>(obj->children["max"]);
        if (max_elem) {
            for (const auto& max_val : max_elem->value) {
                auto str_elem = dynamic_cast<JSONElement<std::string>*>(max_val.get());
                if (str_elem) {
                    uint32_t value;
                    if (acc.componentType == ACCESSOR_FLOAT) {
                        float f = std::stof(str_elem->value);
                        std::memcpy(&value, &f, sizeof(f));
                    } else {
                        value = std::stoi(str_elem->value);
                    }
                    acc.max.push_back(value);
                }
            }
        }
        
        auto min_elem = getElement<std::vector<std::unique_ptr<Element>>>(obj->children["min"]);
        if (min_elem) {
            for (const auto& min_val : min_elem->value) {
                auto str_elem = dynamic_cast<JSONElement<std::string>*>(min_val.get());
                if (str_elem) {
                    uint32_t value;
                    if (acc.componentType == ACCESSOR_FLOAT) {
                        float f = std::stof(str_elem->value);
                        std::memcpy(&value, &f, sizeof(f));
                    } else {
                        value = std::stoi(str_elem->value);
                    }
                    acc.min.push_back(value);
                }
            }
        }
        
        auto name_elem = getElement<std::string>(obj->children["name"]);
        if (name_elem) acc.name = name_elem->value;
        
        // Parse sparse if present
        auto sparse_elem = getElement<JSON>(obj->children["sparse"]);
        if (sparse_elem) {
            JSON* sparse_obj = &sparse_elem->value;
            auto sparse_count = getElement<std::string>(sparse_obj->children["count"]);
            if (sparse_count) acc.sparse.count = std::stoi(sparse_count->value);
            
            auto indices_elem = getElement<JSON>(sparse_obj->children["indices"]);
            if (indices_elem) {
                JSON* indices_obj = &indices_elem->value;
                auto idx_bufferView = getElement<std::string>(indices_obj->children["bufferView"]);
                if (idx_bufferView) acc.sparse.indices.bufferView = std::stoi(idx_bufferView->value);
                auto idx_byteOffset = getElement<std::string>(indices_obj->children["byteOffset"]);
                if (idx_byteOffset) acc.sparse.indices.byteOffset = std::stoi(idx_byteOffset->value);
                auto idx_componentType = getElement<std::string>(indices_obj->children["componentType"]);
                if (idx_componentType) acc.sparse.indices.componentType = std::stoi(idx_componentType->value);
            }
            
            auto values_elem = getElement<JSON>(sparse_obj->children["values"]);
            if (values_elem) {
                JSON* values_obj = &values_elem->value;
                auto val_bufferView = getElement<std::string>(values_obj->children["bufferView"]);
                if (val_bufferView) acc.sparse.values.bufferView = std::stoi(val_bufferView->value);
                auto val_byteOffset = getElement<std::string>(values_obj->children["byteOffset"]);
                if (val_byteOffset) acc.sparse.values.byteOffset = std::stoi(val_byteOffset->value);
            }
        }
        
        accessors.push_back(acc);
    }

    return accessors;
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
    std::cout << JSONToString(&processed);
    // process the buffers
    std::filesystem::path file_pick_path = std::filesystem::path(file_path).parent_path();
    std::vector<std::vector<uint8_t>> read_buffers = readBuffers(buffers, file_pick_path);
    std::vector<buffer_view> read_views = parseBufferViews(bufferViews);
    std::vector<accessor> read_accessors = parseAccessors(accessors);
    return m;
}