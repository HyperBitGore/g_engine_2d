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
#define SAMPLER_NEAREST 9728
#define SAMPLER_LINEAR 9729
#define SAMPLER_NEAREST_MIPMAP_NEAREST 9984
#define SAMPLER_LINEAR_MIPMAP_NEAREST 9985
#define SAMPLER_LINEAR_MIPMAP_NEAREST 9985
#define SAMPLER_NEAREST_MIPMAP_LINEAR 9986
#define SAMPLER_LINEAR_MIPMAP_LINEAR 9987
#define SAMPLER_CLAMP_TO_EDGE 33071
#define SAMPLER_MIRRORED_REPEAT 33648
#define SAMPLER_REPEAT 10497

/*
glTF uses a right-handed coordinate system. glTF defines +Y as up, +Z as forward, and -X as right; the front of a glTF asset faces +Z.
*/

// new flow
//  - if label matches
//      - read string block
//      - feed into parseJSONSection
//      -   recursively parse children of section if subsection { or [
//      -   otherwise add child label
//      -   return the parsed JSON
//  - feed up into readJSONLabel
//  - return said JSON

// TODO support multiple textures
// TODO support model primitives that aren't triangles


std::vector<std::vector<uint8_t>> readBuffers (gore::JSONLoader::JSONFile* json, std::filesystem::path path) {
    // loop through buffers and read
    std::vector<std::vector<uint8_t>> out;
    std::unordered_map<std::string, std::vector<uint8_t>> fileMap;
    gore::JSONArray* buffers = json->getArray("buffers");
    if (!buffers) return out;
    for (auto& i : *buffers) {
        if (i->type == gore::JSONTYPE::OBJECT) {
            auto& elem = gore::JSON::readElementAs<gore::JSON>(i);
            int length = elem.getElementValue<int>("byteLength");
            std::string uri = elem.getElementValue<std::string>( "uri");
            std::vector<uint8_t> p;
            // now read buffer file
            try {
                p = fileMap.at(uri);
            } catch (std::out_of_range e) {
                // ignore and load file
                std::ifstream file(path / std::filesystem::path(uri), std::ios::binary);
                if (!file) {
                    throw std::runtime_error("Malformed uri in gltf file!");
                }
                file.seekg(0, std::ios::end);
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<uint8_t> read(size);
                file.read(reinterpret_cast<char*>(read.data()), size);
                file.close();
                fileMap.emplace(uri, read);
                p = read;
            }
            size_t val = length;
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

std::vector<buffer_view> parseBufferViews (gore::JSONLoader::JSONFile* json) {
    std::vector<buffer_view> out;
    gore::JSONArray* views = json->getArray("bufferViews");
    if (!views) return out;
    for (auto& i : *views) {
        if (i->type != gore::JSONTYPE::OBJECT) continue;
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        buffer_view view;

        view.buffer = (obj.getElementValue<int>("buffer"));
        view.byteLength = (obj.getElementValue<int>("byteLength"));

        auto byteOffset_elem = obj.getElement<int>("byteOffset");
        if (byteOffset_elem) view.byteOffset = (byteOffset_elem->value);

        auto byteStride_elem = obj.getElement<int>("byteStride");
        if (byteStride_elem) view.byteStride = (byteStride_elem->value);

        auto target_elem = obj.getElement<int>("target");
        if (target_elem) view.target = (target_elem->value);

        auto name_elem = obj.getElement<std::string>("name");
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

std::vector<accessor> parseAccessors (gore::JSONLoader::JSONFile* json) {
    std::vector<accessor> accessors;
    gore::JSONArray* elements = json->getArray("accessors");
    if (!elements) return accessors;
    for (auto& i : *elements) {
        if (i->type != gore::JSONTYPE::OBJECT) continue;
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        accessor acc;

        acc.bufferView    = obj.getElementValue<int>("bufferView");
        acc.componentType = obj.getElementValue<int>("componentType");
        acc.count         = obj.getElementValue<int>("count");
        acc.type          = obj.getElementValue<std::string>("type");

        auto byteOffset_elem = obj.getElement<int>("byteOffset");
        if (byteOffset_elem) acc.bufferOffset = byteOffset_elem->value;

        auto normalized_elem = obj.getElement<std::string>("normalized");
        if (normalized_elem) acc.normalized = (normalized_elem->value == "true");

        auto readMinMax = [&](std::string key, std::vector<uint32_t>& out) {
            auto* arr = obj.getArray(key);
            if (!arr) return;
            for (auto& v : *arr) {
                auto str_elem = reinterpret_cast<gore::JSONElement<std::string>*>(v.get());
                if (!str_elem) continue;
                uint32_t value;
                if (acc.componentType == ACCESSOR_FLOAT) {
                    float f = std::stof(str_elem->value);
                    std::memcpy(&value, &f, sizeof(f));
                } else {
                    value = std::stoi(str_elem->value);
                }
                out.push_back(value);
            }
        };
        readMinMax("max", acc.max);
        readMinMax("min", acc.min);

        auto name_elem = obj.getElement<std::string>("name");
        if (name_elem) acc.name = name_elem->value;

        auto sparse_elem = obj.getElement<gore::JSON>("sparse");
        if (sparse_elem) {
            gore::JSON& sparse_obj = sparse_elem->value;
            auto count_elem = sparse_obj.getElement<int>("count");
            if (count_elem) acc.sparse.count = count_elem->value;

            auto indices_elem = sparse_obj.getElement<gore::JSON>("indices");
            if (indices_elem) {
                gore::JSON& idx = indices_elem->value;
                auto bv = idx.getElement<int>("bufferView");
                if (bv) acc.sparse.indices.bufferView = bv->value;
                auto bo = idx.getElement<int>("byteOffset");
                if (bo) acc.sparse.indices.byteOffset = bo->value;
                auto ct = idx.getElement<int>("componentType");
                if (ct) acc.sparse.indices.componentType = ct->value;
            }

            auto values_elem = sparse_obj.getElement<gore::JSON>("values");
            if (values_elem) {
                gore::JSON& vals = values_elem->value;
                auto bv = vals.getElement<int>("bufferView");
                if (bv) acc.sparse.values.bufferView = bv->value;
                auto bo = vals.getElement<int>("byteOffset");
                if (bo) acc.sparse.values.byteOffset = bo->value;
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

std::vector<mesh> parseMeshes (gore::JSONLoader::JSONFile* json) {
    std::vector<mesh> meshes;
    gore::JSONArray* elements = json->getArray("meshes");
    if (!elements) return meshes;
    for (auto& i : *elements) {
        if (i->type != gore::JSONTYPE::OBJECT) continue;
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        mesh m;

        auto name_elem = obj.getElement<std::string>("name");
        if (name_elem) m.name = name_elem->value;

        auto weights_arr = obj.getArray("weights");
        if (weights_arr) {
            for (auto& w : *weights_arr) {
                auto str_elem = reinterpret_cast<gore::JSONElement<std::string>*>(w.get());
                if (str_elem) m.weights.push_back(std::stoi(str_elem->value));
            }
        }

        auto* primitives_arr = obj.getArray("primitives");
        if (primitives_arr) {
            for (auto& prim_val : *primitives_arr) {
                if (prim_val->type != gore::JSONTYPE::OBJECT) continue;
                auto& prim_obj = gore::JSON::readElementAs<gore::JSON>(prim_val);
                mesh_primitive prim;

                auto attributes_elem = prim_obj.getElement<gore::JSON>("attributes");
                if (attributes_elem) {
                    gore::JSON& attr = attributes_elem->value;
                    auto mkAttr = [&](std::string key, MESH_ATTRIBUTES type) {
                        auto e = attr.getElement<int>(key);
                        if (e) prim.attributes.push_back({ type, e->value });
                    };
                    mkAttr("POSITION",   MESH_ATTRIBUTES::POSITION);
                    mkAttr("NORMAL",     MESH_ATTRIBUTES::NORMAL);
                    mkAttr("TANGENT",    MESH_ATTRIBUTES::TANGENT);
                    mkAttr("TEXCOORD_0", MESH_ATTRIBUTES::TEXCOORD);
                    mkAttr("COLOR_0",    MESH_ATTRIBUTES::COLOR);
                    mkAttr("JOINTS_0",   MESH_ATTRIBUTES::JOINTS);
                    mkAttr("WEIGHTS_0",  MESH_ATTRIBUTES::WEIGHTS);
                }

                auto indices_elem = prim_obj.getElement<int>("indices");
                prim.indices = indices_elem ? indices_elem->value : -1;

                auto material_elem = prim_obj.getElement<int>("material");
                if (material_elem) prim.material = material_elem->value;

                auto mode_elem = prim_obj.getElement<int>("mode");
                if (mode_elem) prim.mode = mode_elem->value;

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
    if (index < accessors.size() && index >= 0) {
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
            // byteStride is the distance between the *starts* of consecutive elements.
            // The inner loops already advanced i by (components_at_time * read_at_time) bytes,
            // so only add the remaining padding to reach the next element's start.
            size_t element_byte_size = components_at_time * read_at_time;
            size_t stride_padding = (b.byteStride > -1 && (size_t)b.byteStride > element_byte_size)
                                        ? ((size_t)b.byteStride - element_byte_size)
                                        : 0;
            i  += stride_padding;
            bc += stride_padding;

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

struct texture {
    int sampler;
    int source;
    std::string name;
};

struct image {
    std::string uri;
    std::string mimetype;
    int bufferView;
    std::string name;
};

struct sampler {
    int magFilter = -1;
    int minFilter = -1;
    int wrapS = GL_REPEAT;
    int wrapT = GL_REPEAT;
    std::string name;
};

gore::model gore::model_loader::loadGltf (std::string file_path) {
    gore::JSONLoader::JSONFile json_p = gore::JSONLoader::loadJSONFile(file_path);
    try {
        json_p.getLabel<gore::JSON>("asset");
    } catch (...) {
        throw std::runtime_error("Failed to read asset label, invalid GLTF file! " + file_path);
    }
    std::filesystem::path file_pick_path = std::filesystem::path(file_path).parent_path();
    std::vector<std::vector<uint8_t>> read_buffers = readBuffers(&json_p, file_pick_path);
    std::vector<buffer_view> read_views = parseBufferViews(&json_p);
    std::vector<accessor> read_accessors = parseAccessors(&json_p);
    std::vector<mesh> read_meshes = parseMeshes(&json_p);
    // convert the texture and image arrays to easy struct access
    std::vector<texture> textures;
    gore::JSONArray* texs = json_p.getArray("textures");
    for (auto& i : *texs) {
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        texture t;
        t.sampler = obj.getElementValue<int>("sampler");
        t.source = obj.getElementValue<int>("source");
        t.name = obj.getElementValue<std::string>("name");
        textures.push_back(t);
    }
    std::vector<image> images;
    gore::JSONArray* imgs = json_p.getArray("images");
    for (auto& i : *imgs) {
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        image t;
        t.uri = obj.getElementValue<std::string>("uri");
        t.mimetype = obj.getElementValue<std::string>("mimeType");
        t.bufferView = obj.getElementValue<int>("bufferView");
        t.name = obj.getElementValue<std::string>("name");
        images.push_back(t);
    }
    // sampler
    std::vector<sampler> samplers;
    gore::JSONArray* samples = json_p.getArray("samplers");
    for (auto& i : *samples) {
        auto& obj = gore::JSON::readElementAs<gore::JSON>(i);
        sampler s;
        s.magFilter = obj.getElementValue<int>("magFilter");
        s.minFilter = obj.getElementValue<int>("minFilter");
        s.wrapS = obj.getElementValue<int>("wrapS");
        s.wrapT = obj.getElementValue<int>("wrapT");
        samplers.push_back(s);
    }
    struct temp_mat {
        gore::IMG img;
        int material_index;
    };
    std::vector<temp_mat> mat_images;
    // material
    gore::JSONArray* materials = json_p.getArray("materials");
    int mat_i = 0;
    for (auto& i : *materials) {
        JSON material = JSON::readElementAs<JSON>(i);
        // parse pbr 
        JSON pbr = material.getElementValue<JSON>("pbrMetallicRoughness");
        JSON baseColor = pbr.getElementValue<JSON>("baseColorTexture");
        int index = baseColor.getElementValue<int>("index");
        // get the pbr texture
        auto& texture = textures[index];
        auto& j = images[texture.source];
        // assuming it's a PNG, throw if not
        if (!j.mimetype.empty() && j.mimetype != "image/png") {
            throw std::runtime_error("GLTF image not a PNG, no load!");
        }
        if (j.uri.empty()) {
            throw std::runtime_error("GLTF error image uri is empty!");
        }
        gore::IMG img = gore::imageloader::loadPNG(std::filesystem::path(file_pick_path / j.uri).string());
        // get the sampler and change the texture mode
        auto& l = samplers[texture.sampler];
        glBindTexture(GL_TEXTURE_2D, img->tex);
        if (l.magFilter > -1) {
            glTextureParameteri(img->tex, GL_TEXTURE_MAG_FILTER, l.magFilter);
        }
        if (l.minFilter > -1) {
            glTextureParameteri(img->tex, GL_TEXTURE_MIN_FILTER, l.minFilter);
        }
        glTextureParameteri(img->tex, GL_TEXTURE_WRAP_S, l.wrapS);
        glTextureParameteri(img->tex, GL_TEXTURE_WRAP_T, l.wrapT);
        mat_images.push_back({std::move(img), mat_i});
        mat_i++;
    }
    // now construct the model data
    gore::model m;
    std::vector<gore::vec3> positions;
    std::vector<gore::vec3> normals;
    std::vector<gore::vec2> texcoords;
    std::vector<gore::vec4> tangents;
    std::vector<gore::model_vertex> vertexs;
    std::vector<GLuint> indexs;
    for (auto& mesh : read_meshes) {
        for (auto& prim : mesh.primitives) {
            // attribute indices in this primitive are relative to these bases
            size_t pos_base = positions.size();
            size_t norm_base = normals.size();
            size_t uv_base = texcoords.size();
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
                                        texcoords.push_back({i.x, i.y});
                                    }
                                }
                            break;
                            case ACCESSOR_UNSIGNED_SHORT:
                                {
                                    std::vector<uint16_vec2> new_texs = convertVectorToType<uint16_vec2>(data);
                                    for (auto& i : new_texs) {
                                        float u = (float)i.x / 65535.0f;
                                        float v = (float)i.y / 65535.0f;
                                        // this might be wrong
                                        texcoords.push_back({u, 1.0f - v});
                                    }
                                }
                            break;
                            case ACCESSOR_UNSIGNED_BYTE:
                                {
                                    std::vector<uint8_vec2> new_texs = convertVectorToType<uint8_vec2>(data);
                                    for (auto& i : new_texs) {
                                        float u = (float)i.x / 255.0f;
                                        float v = (float)i.y / 255.0f;
                                        // this also might be wrong
                                        texcoords.push_back({u, 1.0f - v});
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
            const gore::vec2 zero_uv   = {0.0f, 0.0f};
            const gore::vec3 zero_norm = {0.0f, 0.0f, 0.0f};
            // number of vertices this primitive contributed
            size_t prim_vert_count = positions.size() - pos_base;
            bool has_normals = normals.size() > norm_base;
            // convert this primitive's vertices; glTF attribute arrays are
            // already aligned, so vertex i maps straight to model_vertex i
            size_t vert_base = vertexs.size();
            size_t index_base = indexs.size();
            for (size_t i = 0; i < prim_vert_count; i++) {
                gore::model_vertex vert;
                vert.pos = positions[pos_base + i];
                vert.uv = (uv_base + i) < texcoords.size() ? texcoords[uv_base + i] : zero_uv;
                vert.norm = (norm_base + i) < normals.size() ? normals[norm_base + i] : zero_norm;
                vert.material_index = prim.material;
                vertexs.push_back(vert);
            }
            auto addVertexIndex = [&](size_t index, size_t index2, size_t index3) {
                indexs.push_back((GLuint)(vert_base + index));
                indexs.push_back((GLuint)(vert_base + index2));
                indexs.push_back((GLuint)(vert_base + index3));
            };
            // `drawElements()` when defined and `drawArrays()` otherwise
            if (prim.indices == -1) {
               // non-indexed: use all vertices in order, group into triangles
               for (size_t i = 0; i + 2 < prim_vert_count; i += 3) {
                    addVertexIndex(i, i + 1, i + 2);
               }
            } else {
                // indexed: use indices to reference vertices, group into triangles
                std::vector<uint8_t> data = readAccessorData(prim.indices, read_accessors, read_views,  read_buffers);
                int comp_type = read_accessors[prim.indices].componentType;
                // read the number of indices per face at a time
                switch (comp_type) {
                   case ACCESSOR_UNSIGNED_BYTE:
                   {
                        std::vector<uint8_t> index_data = convertVectorToType<uint8_t>(data);
                        for (size_t i = 0; i < index_data.size(); i+=3) {
                            addVertexIndex(index_data[i], index_data[i + 1], index_data[i + 2]);
                        }
                   }
                   break;
                   case ACCESSOR_UNSIGNED_SHORT:
                   {
                        std::vector<uint16_t> index_data = convertVectorToType<uint16_t>(data);
                        for (size_t i = 0; i < index_data.size(); i+=3) {
                            addVertexIndex(index_data[i], index_data[i + 1], index_data[i + 2]);
                        }
                   }
                   break;
                   case ACCESSOR_UNSIGNED_INT:
                   {
                        std::vector<uint32_t> index_data = convertVectorToType<uint32_t>(data);
                        for (size_t i = 0; i < index_data.size(); i+=3) {
                            addVertexIndex(index_data[i], index_data[i + 1], index_data[i + 2]);
                        }
                   }
                   break;
                   default:
                        throw std::runtime_error("The accessor componentType for indices is not an unsigned integer type!");
                }
            }
            if (!has_normals) {
                // compute flat normals from triangle edges for this primitive
                for (size_t i = index_base; i + 2 < indexs.size(); i += 3) {
                    gore::model_vertex& v1 = vertexs[indexs[i]];
                    gore::model_vertex& v2 = vertexs[indexs[i + 1]];
                    gore::model_vertex& v3 = vertexs[indexs[i + 2]];
                    gore::vec3 edge1 = v2.pos - v1.pos;
                    gore::vec3 edge2 = v3.pos - v1.pos;
                    gore::vec3 flat  = edge1.crossProduct(edge2).normalize();
                    v1.norm = v2.norm = v3.norm = flat;
                }
            }
        }
    }
    m = model(gore::index_buffer<gore::model_vertex>(std::move(vertexs), std::move(indexs)), ModelType::GLTF);
    std::vector<gore::model_material::gltf_material> mats_out;
    std::vector<gore::IMG> imgs_out;

    // Helper: read a float field that may be stored as float or int in JSON.
    auto getFloat = [](gore::JSON& obj, const std::string& key, float defval) -> float {
        try { auto* f = obj.getElement<float>(key); if (f) return f->value; } catch (...) {}
        try { auto* n = obj.getElement<int>(key);   if (n) return (float)n->value; } catch (...) {}
        return defval;
    };
    // Helper: read element k of a primitive (string-encoded) array as float.
    auto readArrayFloat = [](gore::JSONArray* arr, size_t idx) -> float {
        if (!arr || idx >= arr->size()) return 0.0f;
        auto* e = reinterpret_cast<gore::JSONElement<std::string>*>((*arr)[idx].get());
        return e ? std::stof(e->value) : 0.0f;
    };

    for (auto& i : mat_images) {
        auto& j = (*materials)[i.material_index];
        auto& mat_json = gore::JSON::readElementAs<gore::JSON>(j);
        model_material::gltf_material mat;

        auto name_elem = mat_json.getElement<std::string>("name");
        if (name_elem) mat.name = name_elem->value;

        // pbrMetallicRoughness block
        auto pbr_elem = mat_json.getElement<gore::JSON>("pbrMetallicRoughness");
        if (pbr_elem) {
            gore::JSON& pbr = pbr_elem->value;

            auto* bcf = pbr.getArray("baseColorFactor");
            if (bcf && bcf->size() >= 4) {
                mat.base_color_factor = {
                    readArrayFloat(bcf, 0), readArrayFloat(bcf, 1),
                    readArrayFloat(bcf, 2), readArrayFloat(bcf, 3)
                };
            }
            mat.metallic_factor  = getFloat(pbr, "metallicFactor",  1.0f);
            mat.roughness_factor = getFloat(pbr, "roughnessFactor", 1.0f);

            auto mrt_elem = pbr.getElement<gore::JSON>("metallicRoughnessTexture");
            if (mrt_elem) {
                auto* idx = mrt_elem->value.getElement<int>("index");
                if (idx) mat.tex_metallic_roughness = idx->value;
            }
        }

        // normalTexture
        auto nt_elem = mat_json.getElement<gore::JSON>("normalTexture");
        if (nt_elem) {
            auto* idx = nt_elem->value.getElement<int>("index");
            if (idx) mat.tex_normal = idx->value;
            mat.normal_scale = getFloat(nt_elem->value, "scale", 1.0f);
        }

        // occlusionTexture
        auto ot_elem = mat_json.getElement<gore::JSON>("occlusionTexture");
        if (ot_elem) {
            auto* idx = ot_elem->value.getElement<int>("index");
            if (idx) mat.tex_occlusion = idx->value;
            mat.occlusion_strength = getFloat(ot_elem->value, "strength", 1.0f);
        }

        // emissiveTexture
        auto et_elem = mat_json.getElement<gore::JSON>("emissiveTexture");
        if (et_elem) {
            auto* idx = et_elem->value.getElement<int>("index");
            if (idx) mat.tex_emissive = idx->value;
        }

        // emissiveFactor
        auto* ef = mat_json.getArray("emissiveFactor");
        if (ef && ef->size() >= 3) {
            mat.emissive_factor = {
                readArrayFloat(ef, 0), readArrayFloat(ef, 1), readArrayFloat(ef, 2)
            };
        }

        // alphaMode ("OPAQUE" / "MASK" / "BLEND")
        auto am_elem = mat_json.getElement<std::string>("alphaMode");
        if (am_elem) {
            if (am_elem->value == "MASK")       mat.alpha_mode = model_material::AlphaMode::ALPHA_MASK;
            else if (am_elem->value == "BLEND") mat.alpha_mode = model_material::AlphaMode::ALPHA_BLEND;
        }
        mat.alpha_cutoff = getFloat(mat_json, "alphaCutoff", 0.5f);

        // doubleSided (JSON boolean stored as string "true"/"false")
        auto ds_elem = mat_json.getElement<std::string>("doubleSided");
        if (ds_elem) mat.double_sided = (ds_elem->value == "true");

        // Associate the pre-loaded base-color image with this material.
        mat.tex_base_color = (int)imgs_out.size();
        imgs_out.push_back(std::move(i.img));

        mats_out.push_back(mat);
    }
    m.addMaterials(mats_out, imgs_out);
    return m;
}