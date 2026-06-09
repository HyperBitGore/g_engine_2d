#include "model_loader.hpp"
#include <stdexcept>

gore::model::model() {
    this->model_matrix = gore::matrix::generateIdentity(4, 4);
    this->image_map.setHashFunction(hash);
}

// we assume that vertexs are in model space still
gore::model::model (std::vector<gore::model_face> faces) {
    this->faces = faces;
    this->model_matrix = matrix::generateIdentity(4, 4);
    this->image_map.setHashFunction(hash);
}
// copy
gore::model::model (const model& m) {
    this->faces = m.faces;
    this->model_matrix = m.model_matrix;
    for (auto& i : m.images) {
        this->images.push_back(imageloader::copyIMG(i));
    }
    this->image_map = m.image_map;
    this->gltfs = m.gltfs;
    this->mtls = m.mtls;
    this->image_map.setHashFunction(hash);
}

// move
gore::model::model (model&& m) {
    this->faces = std::move(m.faces);
    this->model_matrix = std::move(m.model_matrix);
    this->images = std::move(m.images);
    this->image_map = std::move(m.image_map);
    this->gltfs = std::move(m.gltfs);
    this->mtls = std::move(m.mtls);
    this->image_map.setHashFunction(hash);
}

// copy assignment
gore::model& gore::model::operator=(const model& m) {
    this->image_map.setHashFunction(hash);
    if (this != &m) {
        this->faces = m.faces;
        this->model_matrix = m.model_matrix;
        for (auto& i : m.images) {
            this->images.push_back(imageloader::copyIMG(i));
        }
        this->image_map = m.image_map;
        this->gltfs = m.gltfs;
        this->mtls = m.mtls;
    }
    return *this;
}

// move assignment
gore::model& gore::model::operator=(model&& m) {
    this->image_map.setHashFunction(hash);
    if (this != &m) {
        this->faces = std::move(m.faces);
        this->model_matrix = std::move(m.model_matrix);
        this->images = std::move(m.images);
        this->image_map = std::move(m.image_map);
        this->gltfs = std::move(m.gltfs);
        this->mtls = std::move(m.mtls);
    }
    return *this;
}

std::vector<gore::vec3> gore::model::getPositions() const {
    std::vector<gore::vec3> out;
    out.reserve(faces.size() * 3);
    for (const auto& f : faces) {
        out.push_back(f.p1);
        out.push_back(f.p2);
        out.push_back(f.p3);
    }
    return out;
}

std::vector<gore::model_face>& gore::model::getFaces() {
    return faces;
}

void gore::model::addMaterials (const std::vector<model_material::mtl_material>& mats) {
    for (auto& i : mats) {
        // assuming it's a png currently
        if (!i.map_Kd.empty()) {
            IMG img = imageloader::loadPNG(i.map_Kd);
            images.push_back(std::move(img));
            image_map.insert(i.map_Kd, images.size() - 1);
        }
        if (!i.map_Ka.empty()) {
            IMG img = imageloader::loadPNG(i.map_Ka);
            images.push_back(std::move(img));
            image_map.insert(i.map_Ka, images.size() - 1);
        }   
        if (!i.map_Ks.empty()) {
            IMG img = imageloader::loadPNG(i.map_Ks);
            images.push_back(std::move(img));
            image_map.insert(i.map_Ks, images.size() - 1);
        }
        if (!i.map_bump.empty()) {
            IMG img = imageloader::loadPNG(i.map_bump);
            images.push_back(std::move(img));
            image_map.insert(i.map_bump, images.size() - 1);
        }    
        if (!i.map_d.empty()) {
            IMG img = imageloader::loadPNG(i.map_d);
            images.push_back(std::move(img));
            image_map.insert(i.map_d, images.size() - 1);
        }
        mtls.push_back(i);
    }
}

void gore::model::addMaterials (const std::vector<model_material::gltf_material>& mats) {
    for (auto& i : mats) {
        gltfs.push_back(i);
    }
}

gore::IMG& gore::model::getImage (int32_t mtl_index) {
    gore::model_material::mtl_material* mtl = getMTLMat(mtl_index);
    if (mtl) {
        uint32_t* index = image_map.get(mtl->map_Kd);
        if (index) {
            IMG& img = images[*index];
            return img;
        }
    }
    throw std::runtime_error("Image not found for material index: " + std::to_string(mtl_index));
}

gore::model_material::mtl_material* gore::model::getMTLMat (int32_t index) {
    if (index < mtls.size() && index >= 0) {
        return &mtls[index];
    } 
    return nullptr;
}

void gore::model::translate (gore::vec3 translation) {
    model_matrix = model_matrix.translate(translation);
}

void gore::model::rotate (gore::vec3 axis, float radians) {
    model_matrix = model_matrix.rotate(axis, radians);
}

void gore::model::resetMatrix () {
    model_matrix = matrix::generateIdentity(4, 4);
}