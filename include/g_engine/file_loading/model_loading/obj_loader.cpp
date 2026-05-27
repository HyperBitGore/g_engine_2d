#include "model_loader.hpp"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <algorithm>

enum class LineType {VN, VT, V, VP, F, L, UNKNOWN};

std::string consumeNextWord (std::string str, size_t* offset) {
    size_t i = *offset;
    std::string out;
    for (; i < str.size() && (std::isspace(str[i])); i++);
    for (; i < str.size() && !(std::isspace(str[i])); i++) {
        out.push_back(str[i]);
    }
    *offset = i;
    return out;
}

LineType detectLineType (std::string str, size_t* offset) {
    size_t i = *offset;
    std::string out;
    for (; i < str.size() && (std::isspace(str[i])); i++);
    for (; i < str.size() && !(std::isspace(str[i])); i++) {
        out.push_back(str[i]);
    }
    *offset = i;
    if (out == "vn") return LineType::VN;
    if (out == "vt") return LineType::VT;
    if (out == "v")  return LineType::V;
    if (out == "vp") return LineType::VP;
    if (out == "f")  return LineType::F;
    if (out == "l")  return LineType::L;
    return LineType::UNKNOWN;
}

enum class ModelFaceType { POS_ONLY, POS_UV, POS_NORMAL, POS_UV_NORMAL };

struct model_face_index {
    int v1;
    int v2;
    int v3;
    int uv1;
    int uv2;
    int uv3;
    int n1;
    int n2;
    int n3;
};

std::vector<std::string> splitString (std::string str, size_t* offset, std::string delimiter) {
    std::vector<std::string> out;
    size_t i = *offset;
    size_t delim_len = delimiter.size();
    std::string current;
    while (i < str.size()) {
        if (std::isspace(str[i]) && current.size() > 0) {
            out.push_back(current);
            current.clear();
            i++;
        }
        else if (str.substr(i, delim_len) == delimiter) {
            out.push_back(current);
            current.clear();
            i += delim_len;
        } else {
            if (!std::isspace(str[i])) {
                current.push_back(str[i]);
            }
            i++;
        }
    }
    out.push_back(current);
    *offset = i;
    return out;
}

ModelFaceType determineFaceType (std::string vertex) {
    const int c = std::count(vertex.begin(), vertex.end(), '/');
    if (c == 0) return ModelFaceType::POS_ONLY;
    if (c == 1) return ModelFaceType::POS_UV;
    for (size_t i = 0; i < vertex.size() - 1; i++) {
        if (vertex[i] == '/' && vertex[i + 1] == '/') {
            return ModelFaceType::POS_NORMAL;
        }
    }
    return ModelFaceType::POS_UV_NORMAL;
}

model_face_index parseFace (std::string str, size_t* offset) {
    model_face_index out_face = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
    size_t last_offset = *offset;
    std::string type_string = consumeNextWord(str, &last_offset);
    ModelFaceType type = determineFaceType(type_string);
    std::vector<std::string> split = splitString(str, offset, "/");

    auto toIdx = [](const std::string& s) -> int {
        if (s.empty()) return -1;
        return std::stoi(s) - 1; // convert 1-based to 0-based
    };

    switch (type) {
        case ModelFaceType::POS_ONLY:
            // split: [v1, v2, v3]
            if (split.size() >= 3) {
                out_face.v1 = toIdx(split[0]);
                out_face.v2 = toIdx(split[1]);
                out_face.v3 = toIdx(split[2]);
            }
            break;
        case ModelFaceType::POS_UV:
            // split: [v1, vt1, v2, vt2, v3, vt3]
            if (split.size() >= 6) {
                out_face.v1  = toIdx(split[0]); out_face.uv1 = toIdx(split[1]);
                out_face.v2  = toIdx(split[2]); out_face.uv2 = toIdx(split[3]);
                out_face.v3  = toIdx(split[4]); out_face.uv3 = toIdx(split[5]);
            }
            break;
        case ModelFaceType::POS_NORMAL:
            // split: [v1, "", vn1, v2, "", vn2, v3, "", vn3]
            if (split.size() >= 9) {
                out_face.v1 = toIdx(split[0]); out_face.n1 = toIdx(split[2]);
                out_face.v2 = toIdx(split[3]); out_face.n2 = toIdx(split[5]);
                out_face.v3 = toIdx(split[6]); out_face.n3 = toIdx(split[8]);
            }
            break;
        case ModelFaceType::POS_UV_NORMAL:
            // split: [v1, vt1, vn1, v2, vt2, vn2, v3, vt3, vn3]
            if (split.size() >= 9) {
                out_face.v1  = toIdx(split[0]); out_face.uv1 = toIdx(split[1]); out_face.n1 = toIdx(split[2]);
                out_face.v2  = toIdx(split[3]); out_face.uv2 = toIdx(split[4]); out_face.n2 = toIdx(split[5]);
                out_face.v3  = toIdx(split[6]); out_face.uv3 = toIdx(split[7]); out_face.n3 = toIdx(split[8]);
            }
            break;
    }
    return out_face;
}

// need texture/mtl support
// need line element support
// need quad/ngon support (maybe)
// need parameter space support
// https://paulbourke.net/dataformats/obj/
// https://en.wikipedia.org/wiki/Wavefront_.obj_file
gore::model gore::model_loader::loadObj (std::string file_path) {
    std::ifstream file(file_path);
    if (!file) {
        throw std::runtime_error("Invalid OBJ file path " + file_path);
    }
    std::string line;
    std::vector<std::string> lines;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    std::vector<gore::vec3> positions;
    std::vector<gore::vec2> uvs;
    std::vector<gore::vec3> normals;
    std::vector<model_face> faces;

    const gore::vec2 zero_uv   = {0.0f, 0.0f};
    const gore::vec3 zero_norm = {0.0f, 0.0f, 0.0f};

    for (auto& i : lines) {
        if (i.empty() || i[0] == '#') continue;
        size_t offset = 0;
        LineType type = detectLineType(i, &offset);
        switch (type) {
            case LineType::V: {
                std::string x = consumeNextWord(i, &offset);
                std::string y = consumeNextWord(i, &offset);
                std::string z = consumeNextWord(i, &offset);
                if (!x.empty() && !y.empty() && !z.empty())
                    positions.push_back({std::stof(x), std::stof(y), std::stof(z)});
                break;
            }
            case LineType::VT: {
                std::string u = consumeNextWord(i, &offset);
                std::string v = consumeNextWord(i, &offset);
                if (!u.empty() && !v.empty())
                    uvs.push_back({std::stof(u), 1.0f - std::stof(v)}); // flip V for OpenGL
                break;
            }
            case LineType::VN: {
                std::string x = consumeNextWord(i, &offset);
                std::string y = consumeNextWord(i, &offset);
                std::string z = consumeNextWord(i, &offset);
                if (!x.empty() && !y.empty() && !z.empty())
                    normals.push_back({std::stof(x), std::stof(y), std::stof(z)});
                break;
            }
            case LineType::F: {
                model_face_index idx = parseFace(i, &offset);
                gore::model_face face;
                face.p1 = positions[idx.v1];
                face.p2 = positions[idx.v2];
                face.p3 = positions[idx.v3];
                face.uv1 = idx.uv1 >= 0 ? uvs[idx.uv1] : zero_uv;
                face.uv2 = idx.uv2 >= 0 ? uvs[idx.uv2] : zero_uv;
                face.uv3 = idx.uv3 >= 0 ? uvs[idx.uv3] : zero_uv;
                if (idx.n1 >= 0 && idx.n2 >= 0 && idx.n3 >= 0) {
                    face.norm1 = normals[idx.n1];
                    face.norm2 = normals[idx.n2];
                    face.norm3 = normals[idx.n3];
                } else {
                    // compute flat normal from triangle edges
                    gore::vec3 edge1 = face.p2 - face.p1;
                    gore::vec3 edge2 = face.p3 - face.p1;
                    gore::vec3 flat  = edge1.crossProduct(edge2).normalize();
                    face.norm1 = face.norm2 = face.norm3 = flat;
                }
                faces.push_back(face);
                break;
            }
            default:
                break;
        }
    }
    return model(faces);
}