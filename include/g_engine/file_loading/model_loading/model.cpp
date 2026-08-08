#include "model_loader.hpp"
#include <GL/gl.h>
#include <stdexcept>

gore::model::model() : type(ModelType::UNLOADED) {
    this->model_matrix = gore::matrix::generateIdentity(4, 4);
    this->image_map.setHashFunction(hash);
}
gore::model::~model() {
    for (auto& i : images) {
        glDeleteTextures(1, &i->tex);
    }
}
// we assume that vertexs are in model space still
gore::model::model (gore::index_buffer<model_vertex> buffer, const ModelType t) : type(t) {
    this->index_buffer = buffer;
    this->model_matrix = matrix::generateIdentity(4, 4);
    this->image_map.setHashFunction(hash);
}
// copy
gore::model::model (const model& m) : type(m.type) {
    this->image_map.setHashFunction(hash);
    this->model_matrix = m.model_matrix;
    for (auto& i : m.images) {
        this->images.push_back(imageloader::copyIMG(i));
    }
    this->image_map = m.image_map;
    this->gltfs = m.gltfs;
    this->mtls = m.mtls;
    this->index_buffer = m.index_buffer;
    this->texture_count = m.texture_count;
}

// move
gore::model::model (model&& m) noexcept : type(m.type) {
    this->image_map.setHashFunction(hash);
    this->model_matrix = std::move(m.model_matrix);
    this->images = std::move(m.images);
    this->image_map = std::move(m.image_map);
    this->gltfs = std::move(m.gltfs);
    this->mtls = std::move(m.mtls);
    this->index_buffer = std::move(m.index_buffer);
    this->texture_count = std::move(m.texture_count);
}

// copy assignment
gore::model& gore::model::operator=(const model& m) {
    this->image_map.setHashFunction(hash);
    if (this != &m) {
        this->model_matrix = m.model_matrix;
        for (auto& i : m.images) {
            this->images.push_back(imageloader::copyIMG(i));
        }
        this->image_map = m.image_map;
        this->gltfs = m.gltfs;
        this->mtls = m.mtls;
        this->type = m.type;
        this->index_buffer = m.index_buffer;
        this->texture_count = m.texture_count;
    }
    return *this;
}

// move assignment
gore::model& gore::model::operator=(model&& m) noexcept {
    this->image_map.setHashFunction(hash);
    if (this != &m) {
        this->model_matrix = std::move(m.model_matrix);
        this->images = std::move(m.images);
        this->image_map = std::move(m.image_map);
        this->gltfs = std::move(m.gltfs);
        this->mtls = std::move(m.mtls);
        this->type = m.type;
        this->index_buffer = std::move(m.index_buffer);
        this->texture_count = std::move(m.texture_count);
    }
    return *this;
}

std::vector<gore::vec3> gore::model::getPositions() const {
    std::vector<gore::vec3> out;
    out.reserve(index_buffer.vertexSize());
    for (const auto& f : index_buffer.getVertexs()) {
        out.push_back(f.pos);
    }
    return out;
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
        texture_count++;
    }
}

void gore::model::addMaterials (std::vector<gore::model_material::gltf_material>& mats) {
    for (auto& i : mats) {
        gltfs.push_back(i);
        texture_count++;
    }
}

void gore::model::addMaterials (std::vector<gore::model_material::gltf_material>& mats, std::vector<gore::IMG>& imgs) {
    for (size_t k = 0; k < mats.size(); k++) {
        if (k < imgs.size()) {
            std::string key = mats[k].name.empty()
                ? "gltf_mat_" + std::to_string(k)
                : mats[k].name;
            images.push_back(std::move(imgs[k]));
            image_map.insert(key, (uint32_t)(images.size() - 1));
        }
        gltfs.push_back(mats[k]);
        texture_count++;
    }
}

gore::IMG& gore::model::getImage (int32_t mat_index) {
    if (this->type == ModelType::OBJ) {
        gore::model_material::mtl_material* mtl = getMTLMat(mat_index);
        if (mtl) {
            uint32_t* index = image_map.get(mtl->map_Kd);
            if (index) {
                IMG& img = images[*index];
                return img;
            }
        }
    } else if (this->type == ModelType::GLTF) {
        gore::model_material::gltf_material* gltf = getGLTFMat(mat_index);
        if (gltf) {
            if (gltf->tex_base_color > -1) {
                IMG& img = images[gltf->tex_base_color];
                return img;
            }
        }
    } else {
        throw std::runtime_error("MODEL not loaded!");
    }
    throw std::runtime_error("Image not found for material index: " + std::to_string(mat_index));
}

std::vector<gore::IMG>& gore::model::getImages () {
    return images;
}

gore::model_material::mtl_material* gore::model::getMTLMat (int32_t index) {
    if (index < mtls.size() && index >= 0) {
        return &mtls[index];
    } 
    return nullptr;
}

gore::model_material::gltf_material* gore::model::getGLTFMat (int32_t index) {
     if (index < gltfs.size() && index >= 0) {
        return &gltfs[index];
    } 
    return nullptr;
}

void gore::model::addImageMaterialMTL(IMG img, const std::string& key) {
    model_material::mtl_material mat;
    mat.name   = key;
    mat.map_Kd = key;
    img->name  = key;
    images.push_back(std::move(img));
    image_map.insert(mat.map_Kd, (uint32_t)(images.size() - 1));
    mtls.push_back(std::move(mat));
    texture_count++;
}

void gore::model::translate (gore::vec3 translation) {
    model_matrix = model_matrix.translate(translation);
}

void gore::model::rotate (gore::vec3 axis, float radians) {
    model_matrix = model_matrix.rotate(axis, radians);
}

void gore::model::scale (gore::vec3 scale) {
    model_matrix = model_matrix.scale(scale);
}

void gore::model::resetMatrix () {
    model_matrix = matrix::generateIdentity(4, 4);
}