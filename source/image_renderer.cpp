#include "g_engine_2d.h"

void ImageRenderer::addImageVertex(float x, float y, float w, float h){

}

void ImageRenderer::drawBuffer(){

}

void ImageRenderer::drawImage(IMG img, float x, float y, float w, float h){
    vertexs.push_back({x, y, 0.0f, 0.0f}); //first triangle top left vertex
    vertexs.push_back({x + w, y, 1.0f, 0.0f}); //first triangel top right
    vertexs.push_back({x, y + h, 0.0f, 1.0f}); //first triangle tip vertex

    vertexs.push_back({x + w, y + h, 1.0f, 1.0f});
    vertexs.push_back({x, y + h, 0.0f, 1.0f});
    vertexs.push_back({x + w, y, 1.0f, 0.0f});
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

ImageRenderer::ImageRenderer(size_t w, size_t h) {
    shader.compile(std::string("img.vs"), std::string("img.fs"));
    glGenVertexArrays_g(1, &vao);
    glGenBuffers_g(1, &vertex_buffer);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray_g(0);
    glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0);
    glEnableVertexAttribArray_g(1);
    glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2));
    shader.setuniform("screen", {(float)w, (float)h});
}