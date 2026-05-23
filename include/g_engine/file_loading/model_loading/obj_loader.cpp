#include "model_loader.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

std::string consumeNextWord (std::string str, size_t* offset) {
    size_t i = *offset;
    std::string out;
    for (; i < str.size() && (str[i] == ' ' || str[i] == '\t'); i++);
    for (; i < str.size() && !(str[i] == ' ' || str[i] == '\t'); i++) {
        out.push_back(str[i]);
    }
    *offset = i;
    return out;
}


// https://paulbourke.net/dataformats/obj/
// https://en.wikipedia.org/wiki/Wavefront_.obj_file
// currently only outputs vertexs
std::vector<gore::vec3> gore::model_loader::loadObj (std::string file_path) {
    std::ifstream file(file_path);
    if (!file) {
        throw std::runtime_error("Invalid OBJ file path " + file_path);
    }
    // throw obj file into memory
    std::string line;
    std::vector<std::string> lines;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    // parse vertexs
    std::vector<gore::vec3> vertexs;
    for (auto& i : lines) {
        if (i[0] == '#'){
            continue;
        }
        switch (i[0]) {
            case '#':
            continue;
            case 'v':
                // process the line
                {
                    if (i[1] == ' ') {
                        size_t offset = 1;
                        std::string p1 = consumeNextWord(i, &offset);
                        std::string p2 = consumeNextWord(i, &offset);
                        std::string p3 = consumeNextWord(i, &offset);
                        vertexs.push_back({std::stof(p1), std::stof(p2), std::stof(p3)});
                    }
                }
            break;
            default:
            continue;
        }

    }
    gore::model model = gore::model(vertexs);
    return vertexs;
}