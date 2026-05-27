#include "model_loader.hpp"

// we assume that vertexs are in model space still
gore::model::model (std::vector<vec3> vertexs) {
    this->vertexs = vertexs;
    this->model_matrix = matrix::generateIdentity(this->model_matrix.numRows(), this->model_matrix.numColumns());
}
// copy
gore::model::model (const model& m) {
    this->vertexs = m.vertexs;
    this->model_matrix = m.model_matrix;
}