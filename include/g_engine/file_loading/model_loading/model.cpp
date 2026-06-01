#include "model_loader.hpp"

gore::model::model() {
    this->model_matrix = gore::matrix::generateIdentity(4, 4);
}

// we assume that vertexs are in model space still
gore::model::model (std::vector<gore::model_face> faces) {
    this->faces = faces;
    this->model_matrix = matrix::generateIdentity(this->model_matrix.numRows(), this->model_matrix.numColumns());
}
// copy
gore::model::model (const model& m) {
    this->faces = m.faces;
    this->model_matrix = m.model_matrix;
    this->img = m.img;
    this->gltfs = m.gltfs;
    this->mtls = m.mtls;
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
        mtls.push_back(i);
    }
}

void gore::model::addMaterials (const std::vector<model_material::gltf_material>& mats) {
    for (auto& i : mats) {
        gltfs.push_back(i);
    }
}