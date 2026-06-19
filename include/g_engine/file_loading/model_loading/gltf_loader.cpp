#include "model_loader.hpp"
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#define ACCESSOR_SIGNED_BYTE 5120
#define ACCESSOR_UNSIGNED_BYTE 5121
#define ACCESSOR_SIGNED_SHORT 5122
#define ACCESSOR_UNSIGNED_SHORT 5123
#define ACCESSOR_UNSIGNED_INT 5125
#define ACCESSOR_FLOAT 5126
#define ARRAY_BUFFER 34962
#define ELEMENT_ARRAY_BUFFER 34963
#define MESH_MODE_POINTS 0
#define MESH_MODE_LINES 1
#define MESH_MODE_LINE_LOOP 2
#define MESH_MODE_LINE_STRIP 3
#define MESH_MODE_TRIANGLES 4
#define MESH_MODE_TRIANGLE_STRIP 5
#define MESH_MODE_TRIANGLE_FAN 6

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

enum class MESH_ATTRIBUTES { POSITION, NORMAL, TANGENT, TEXCOORD, COLOR, JOINTS, WEIGHTS};

struct mesh_attribute {
    MESH_ATTRIBUTES attrib;
    int index;
};

struct mesh_primitive {
    std::vector<mesh_attribute> attributes;
    int indices;
    int material;
    int mode = 4;
};

struct mesh {
    std::vector<mesh_primitive> primitives;
    std::vector<int> weights;
    std::string name;
};

std::vector<mesh> parseMeshes (std::vector<std::unique_ptr<Element>>* elements) {
    std::vector<mesh> meshes;
    for (auto& i : *elements) {
        auto obj_elem = dynamic_cast<JSONElement<JSON>*>(i.get());
        if (!obj_elem) continue;
        
        mesh m;
        JSON* obj = &obj_elem->value;
        
        // Parse name
        auto name_elem = getElement<std::string>(obj->children["name"]);
        if (name_elem) m.name = name_elem->value;
        
        // Parse weights
        auto weights_elem = getElement<std::vector<std::unique_ptr<Element>>>(obj->children["weights"]);
        if (weights_elem) {
            for (const auto& weight_val : weights_elem->value) {
                auto str_elem = dynamic_cast<JSONElement<std::string>*>(weight_val.get());
                if (str_elem) {
                    m.weights.push_back(std::stoi(str_elem->value));
                }
            }
        }
        
        // Parse primitives
        auto primitives_elem = getElement<std::vector<std::unique_ptr<Element>>>(obj->children["primitives"]);
        if (primitives_elem) {
            for (const auto& prim_val : primitives_elem->value) {
                auto prim_elem = dynamic_cast<JSONElement<JSON>*>(prim_val.get());
                if (!prim_elem) continue;
                
                mesh_primitive prim;
                JSON* prim_obj = &prim_elem->value;
                
                // Parse attributes
                auto attributes_elem = getElement<JSON>(prim_obj->children["attributes"]);
                if (attributes_elem) {
                    JSON* attr_obj = &attributes_elem->value;
                    
                    // Check for POSITION
                    auto position_elem = getElement<std::string>(attr_obj->children["POSITION"]);
                    if (position_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::POSITION;
                        attr.index = std::stoi(position_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for NORMAL
                    auto normal_elem = getElement<std::string>(attr_obj->children["NORMAL"]);
                    if (normal_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::NORMAL;
                        attr.index = std::stoi(normal_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for TANGENT
                    auto tangent_elem = getElement<std::string>(attr_obj->children["TANGENT"]);
                    if (tangent_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::TANGENT;
                        attr.index = std::stoi(tangent_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for TEXCOORD_0
                    auto texcoord_elem = getElement<std::string>(attr_obj->children["TEXCOORD_0"]);
                    if (texcoord_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::TEXCOORD;
                        attr.index = std::stoi(texcoord_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for COLOR_0
                    auto color_elem = getElement<std::string>(attr_obj->children["COLOR_0"]);
                    if (color_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::COLOR;
                        attr.index = std::stoi(color_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for JOINTS_0
                    auto joints_elem = getElement<std::string>(attr_obj->children["JOINTS_0"]);
                    if (joints_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::JOINTS;
                        attr.index = std::stoi(joints_elem->value);
                        prim.attributes.push_back(attr);
                    }
                    
                    // Check for WEIGHTS_0
                    auto weights_attr_elem = getElement<std::string>(attr_obj->children["WEIGHTS_0"]);
                    if (weights_attr_elem) {
                        mesh_attribute attr;
                        attr.attrib = MESH_ATTRIBUTES::WEIGHTS;
                        attr.index = std::stoi(weights_attr_elem->value);
                        prim.attributes.push_back(attr);
                    }
                }
                
                // Parse indices
                auto indices_elem = getElement<std::string>(prim_obj->children["indices"]);
                if (indices_elem) { prim.indices = std::stoi(indices_elem->value); } else { prim.indices = -1; }
                
                // Parse material
                auto material_elem = getElement<std::string>(prim_obj->children["material"]);
                if (material_elem) prim.material = std::stoi(material_elem->value);
                
                // Parse mode (default is 4 for triangles)
                auto mode_elem = getElement<std::string>(prim_obj->children["mode"]);
                if (mode_elem) prim.mode = std::stoi(mode_elem->value);
                
                m.primitives.push_back(prim);
            }
        }
        
        meshes.push_back(m);
    }
    return meshes;
}

enum class ACCESSOR_TYPE { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };

ACCESSOR_TYPE stringToAccessorType (std::string str) {
    if (str == "SCALAR") {
        return ACCESSOR_TYPE::SCALAR;
    } else if (str == "VEC2") {
        return ACCESSOR_TYPE::VEC2;
    } else if (str == "VEC3") {
        return ACCESSOR_TYPE::VEC3;
    } else if (str == "VEC4") {
        return ACCESSOR_TYPE::VEC4;
    } else if (str == "MAT2") {
        return ACCESSOR_TYPE::MAT2;
    } else if (str == "MAT3") {
        return ACCESSOR_TYPE::MAT3;
    } else if (str == "MAT4") {
        return ACCESSOR_TYPE::MAT4;
    }
    return ACCESSOR_TYPE::SCALAR;
}

uint32_t getAccessorTypeBytes (int type) {
    uint32_t read_at_time = 1;
    switch (type) {
        case ACCESSOR_SIGNED_BYTE:
        case ACCESSOR_UNSIGNED_BYTE:
        break;
        case ACCESSOR_UNSIGNED_INT:
        case ACCESSOR_FLOAT:
        read_at_time = 4;
        break;
        case ACCESSOR_UNSIGNED_SHORT:
        case ACCESSOR_SIGNED_SHORT:
        read_at_time = 2;
        break;
    }
    return read_at_time;
}

std::vector<uint8_t> readAccessorData (int index, std::vector<accessor>& accessors, std::vector<buffer_view>& buffer_views, std::vector<std::vector<uint8_t>> buffers) {
    std::vector<uint8_t> buffer;
    if (index < accessors.size() && index > 0) {
        accessor a = accessors[index];
        buffer_view b = buffer_views[a.bufferView];
        std::vector<uint8_t> bf = buffers[b.buffer];
        uint32_t read_at_time = getAccessorTypeBytes(a.componentType);
        ACCESSOR_TYPE type = stringToAccessorType(a.type);
        uint32_t components_at_time = 1;
        switch (type) {
            case ACCESSOR_TYPE::SCALAR:
            break;
            case ACCESSOR_TYPE::VEC2:
                components_at_time = 2;
            break;
            case ACCESSOR_TYPE::VEC3:
                components_at_time = 3;
            break;
            case ACCESSOR_TYPE::VEC4:
                components_at_time = 4;
            break;
            case ACCESSOR_TYPE::MAT2:
                components_at_time = 4;
            break;
            case ACCESSOR_TYPE::MAT3:
                components_at_time = 9;
            break;
            case ACCESSOR_TYPE::MAT4:
                components_at_time = 16;
            break;
        }
        size_t c = 0;
        size_t bc = 0;
        for (size_t i = a.bufferOffset + b.byteOffset; i < bf.size() && bc < b.byteLength && c < a.count; c++) {
            // concat bytes for it
            std::vector<uint8_t> bytes;
            for (size_t cc = 0; cc < components_at_time; cc++) {
                uint32_t data = 0;
                for (size_t bb = 0; bb < read_at_time; bb++) {
                    if (i < bf.size()) {
                        data |= (static_cast<uint32_t>(bf[i]) << (bb * 8));
                        i++;
                        bc++;
                    }
                }
                bytes.push_back(static_cast<uint8_t>(data & 0xFF));
                if (read_at_time > 1) bytes.push_back(static_cast<uint8_t>((data >> 8) & 0xFF));
                if (read_at_time > 2) bytes.push_back(static_cast<uint8_t>((data >> 16) & 0xFF));
                if (read_at_time > 3) bytes.push_back(static_cast<uint8_t>((data >> 24) & 0xFF));
            }
            for (const auto& byte : bytes) {
                buffer.push_back(byte);
            }
            // add byte stride
            i += (b.byteStride > -1) ? b.byteStride : 0;

        }
    }
    return buffer;
}

template <class G>
std::vector<G> convertVectorToType (std::vector<uint8_t> data) {
    static_assert(std::is_trivially_copyable_v<G>);
    uint8_t* read_data = reinterpret_cast<uint8_t*>(data.data());
    std::vector<G> new_data;
    for (size_t i = 0; i < data.size(); i += sizeof(G)) {
        G g;
        std::memcpy((void*)&g, read_data + i, sizeof(G));
        new_data.push_back(g);
    }
    return new_data;
}

void testConvertVectorToType(const std::vector<uint8_t>& data) {
    std::vector<gore::vec3> positions;
    std::ofstream f("test.txt");
    std::vector<float> float_data = convertVectorToType<float>(data);
    f << "POSITION: positions vector size: " << positions.size() << "\n";
    for (size_t i = 0; i < float_data.size(); i+=3) {
        gore::vec3 vec = { float_data[i], float_data[i + 1], float_data[i + 2] };
        positions.push_back(vec);
        f << "Index " << i << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
    }
    f.close();
    
    f.open("test2.txt");
    std::vector<gore::vec3> new_positions = convertVectorToType<gore::vec3>(data);
    f << "new_positions: Read " << new_positions.size() << " vertices\n";
    for (auto& i : new_positions) {
        f << "position: (" << i.x << ", " << i.y << ", " << i.z << ")\n";
    }
    f.close();
    
    f.open("test3.txt");
    f << "Comparing positions and new_positions:\n";
    for (size_t i = 0; i < positions.size(); i++) {
        auto& p1 = positions[i];
        auto& p2 = new_positions[i];
        bool same = (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z);
        f << "Index " << i << ": (" << p1.x << ", " << p1.y << ", " << p1.z << ") vs (" 
          << p2.x << ", " << p2.y << ", " << p2.z << ") - same: " << (same ? "true" : "false") << "\n";
    }
    f.close();
}

struct uint16_vec2 {
    uint16_t x;
    uint16_t y;
};


struct uint8_vec2 {
    uint8_t x;
    uint8_t y;
};

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
    std::vector<std::unique_ptr<Element>>* meshes = getLabelArray(&processed, "meshes");
    std::cout << JSONToString(&processed);
    // process the buffers
    std::filesystem::path file_pick_path = std::filesystem::path(file_path).parent_path();
    std::vector<std::vector<uint8_t>> read_buffers = readBuffers(buffers, file_pick_path);
    std::vector<buffer_view> read_views = parseBufferViews(bufferViews);
    std::vector<accessor> read_accessors = parseAccessors(accessors);
    std::vector<mesh> read_meshes = parseMeshes(meshes);
    // now construct the model data
    gore::model m;
    std::vector<gore::vec3> positions;
    std::vector<gore::vec3> normals;
    std::vector<gore::vec2> texcoords;
    std::vector<gore::vec4> tangents;
    // TODO support multiple textures
    for (auto& mesh : read_meshes) {
        for (auto& prim : mesh.primitives) {
            for (auto& attrib : prim.attributes) {
                std::vector<uint8_t> data = readAccessorData(attrib.index, read_accessors, read_views,  read_buffers);
                int comp_type = read_accessors[attrib.index].componentType;
                switch (attrib.attrib) {
                case MESH_ATTRIBUTES::POSITION:
                    // now read as a vec3
                    {   
                        // testConvertVectorToType(data);
                        std::vector<gore::vec3> new_positions = convertVectorToType<gore::vec3>(data);
                        for (auto& i : new_positions) {
                            positions.push_back(i);
                        }
                    }
                break;
                case MESH_ATTRIBUTES::NORMAL:
                    {
                        std::vector<gore::vec3> new_normals = convertVectorToType<gore::vec3>(data);
                        for (auto& i : new_normals) {
                            normals.push_back(i);
                        }
                    }
                break;
                case MESH_ATTRIBUTES::TANGENT:
                    {
                         std::vector<gore::vec4> new_tangents = convertVectorToType<gore::vec4>(data);
                        for (auto& i : new_tangents) {
                            tangents.push_back(i);
                        }
                    }
                break;
                case MESH_ATTRIBUTES::TEXCOORD:
                    {
                        switch (comp_type) {
                            case ACCESSOR_FLOAT:
                                {
                                    std::vector<gore::vec2> new_texs = convertVectorToType<gore::vec2>(data);
                                    for (auto& i : new_texs) {
                                        texcoords.push_back(i);
                                    }
                                }
                            break;
                            case ACCESSOR_UNSIGNED_SHORT:
                                {
                                    // cast to some type of unsigned short version of tex
                                    std::vector<uint16_vec2> new_texs = convertVectorToType<uint16_vec2>(data);
                                    for (auto& i : new_texs) {
                                        // supposed to be normalized
                                         texcoords.push_back({ 
                                            (float)i.x / 65535.0f, 
                                            (float)i.y / 65535.0f
                                        });
                                    }
                                }
                            break;
                            case ACCESSOR_UNSIGNED_BYTE:
                                {
                                    std::vector<uint8_vec2> new_texs = convertVectorToType<uint8_vec2>(data);
                                    for (auto& i : new_texs) {
                                        texcoords.push_back({ 
                                            (float)i.x / 255.0f, 
                                            (float)i.y / 255.0f
                                        });
                                    }
                                }
                            break;
                        }
                    }
                break;
                case MESH_ATTRIBUTES::COLOR:
                    {
                        // TODO
                    }
                break;
                case MESH_ATTRIBUTES::JOINTS:
                    {
                        // TODO
                    }
                break;
                case MESH_ATTRIBUTES::WEIGHTS:
                    {
                        // TODO
                    }
                    break;
                }
            }
            // `drawElements()` when defined and `drawArrays()` otherwise
            if (prim.indices == -1) {
                // non-indexed
                // TODO
            } else {
                // indexed
                // get the accessor data
                std::vector<uint8_t> data = readAccessorData(prim.indices, read_accessors, read_views,  read_buffers);
                int comp_type = read_accessors[prim.indices].componentType;
                switch (comp_type) {
                    case ACCESSOR_UNSIGNED_BYTE:
                    case ACCESSOR_UNSIGNED_INT:
                    case ACCESSOR_UNSIGNED_SHORT:
                    break;
                    default:
                    throw std::runtime_error("The accessor componentType is not an unsigned int type!");
                }
            }
        }
    }
    return m;
}