#include "renderer.hpp"

template<class T>
gore::renderer<T>::renderer (std::string vertex_shader, std::string fragment_shader, uint32_t width, uint32_t height) {
    vertexs.reserve(1000);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    allocated = 1;
    glGenBuffers(1, &vertex_buffer);
    shader.compile(vertex_shader.c_str(), fragment_shader.c_str());
    shader.bind();
    this->width = width;
    this->height = height;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    shader_setup();
}
template<class T>
void gore::renderer<T>::drawBuffer () {
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(draw_arrays_mode, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
template<class T>
void gore::renderer<T>::setDimensions (uint32_t width, int32_t height) {
    matrix ortho = matrix::calculateOrtho(width, height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    this->width = width;
    this->height = height;
}
template<class T>
void gore::renderer<T>::updateView (float x, float y, float zoom) {
    matrix view = matrix::calculate2DView(x, y, zoom);
    shader.setuniform("view", 1, true, view);
}