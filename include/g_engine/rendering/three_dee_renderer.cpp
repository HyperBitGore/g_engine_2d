#include "three_dee_renderer.hpp"
#include "three_dee_renderer_shader.hpp"

void gore::threedeerender::shader_setup()  {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gore::threedee_vertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 3)); //uvs
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 5)); // model matrice index
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 6)); // texture unit index
    updateDimensions(this->width, this->height);
    updateView({0, 0, 5}, {0, 0, 0}, gore::vec3(0,1,0));
    shader.setuniform("set_color", {1.0f, 1.0f, 1.0f, 1.0f});
}

void gore::threedeerender::updateDimensions (uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;
    gore::matrix perspective = gore::matrix::perspective(toRadians(this->vertical_fov), (float)this->width / (float)this->height, this->near_clip, this->far_clip);
    shader.setuniform("projection", 1, true, perspective);
}

gore::threedeerender::threedeerender(size_t w, size_t h) : gore::renderer<gore::threedeerender, gore::threedee_vertex> (three_dee_renderer_vertex, three_dee_renderer_fragment, w, h) {

}
// unskinned vertex
void gore::threedeerender::addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3) {
    vertexs.push_back({pos.x, pos.y, pos.z, 0.0, 0.0, -1, -1});
    vertexs.push_back({pos2.x, pos2.y, pos2.z, 0.0, 0.0, -1, -1});
    vertexs.push_back({pos3.x, pos3.y, pos3.z, 0.0, 0.0, -1, -1});
}
// unskinned vertexs
void gore::threedeerender::addVertexs(const std::vector<gore::vec3>& vertexs) {
    for (auto& i : vertexs) {
        this->vertexs.push_back({i.x, i.y, i.z, 0.0, 0.0, -1, -1});
    }
}
void gore::threedeerender::drawBuffer() {
    assert(created && "call createRenderer before use!");
    if (vertexs.empty()) return;
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(threedee_vertex), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(threedee_vertex), &vertexs[0]);
    }
    glDrawArraysExt(draw_arrays_mode, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}