#include "image_loader.hpp"
#include <fstream>
#include <filesystem>


IMG imageloader::loadPNG(std::string path, unsigned int w, unsigned int h) {
    // open file
    std::ifstream file;
    std::vector<char> buffer;
    file.open(path, std::ios::binary);
    uintmax_t file_size = std::filesystem::file_size(path);
    file.close();
    if (file_size < 67) {
        // below minimum size
        return nullptr;
    }
    // read the 
}