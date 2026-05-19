#include "image_renderer.hpp"
#include "image_renderer_shader.hpp"
#include <GL/gl.h>

GLuint gore::imagerenderer::getTextureUnit (GLuint texture) {
    GLuint* unit = texture_unit_map.get(texture);
    if (unit == nullptr) {
        uint32_t f_unit = GL_TEXTURE0 + current_unit;
        texture_unit_map.insert(texture, current_unit);
        glActiveTexture(f_unit);
        glBindTexture(GL_TEXTURE_2D, texture);
        samplers.push_back(current_unit);
        current_unit += 1;
        return current_unit - 1;
        
    }
    return *unit;
}

void gore::imagerenderer::setTextureSamplers () {
    // issue is the setting of uniform mtexture why the indexing doesn't work, fix that here
    shader.setuniform("mtexture", samplers.size(), samplers.data());
}

void gore::imagerenderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim){
    addImageVertex(texture, pos, dim, {0, 0, 1.0, 1.0}, 0.0f);
}

void gore::imagerenderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim, float rot){
    addImageVertex(texture, pos, dim, {0, 0, 1.0, 1.0}, rot);
}
//first two x,y in uv is starting position in image and z, w are width and height for the uvs
void gore::imagerenderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim, vec4 uvs, float rot){
    GLuint unit = getTextureUnit(texture);
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y, unit}); //top righjt
}

void gore::imagerenderer::drawBuffer() {
    setTextureSamplers();
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArraysExt(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
    texture_unit_map.clear();
    samplers.clear();
    this->current_unit = 0;
}

void gore::imagerenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim){
    drawTexture(texture, pos, dim, {0, 0, 1.0, 1.0});
}
void gore::imagerenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim, vec4 uvs){
    GLuint unit = getTextureUnit(texture);
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y, unit}); //top righjt
    drawBuffer();
}

void gore::imagerenderer::drawImage(IMG img, vec2 pos, vec2 dim){
    drawImage(img, pos, dim, {0, 0, 1.0, 1.0});
}

void gore::imagerenderer::drawImage(IMG img, vec2 pos, vec2 dim, vec4 uvs){
    GLuint unit = getTextureUnit(img->tex);
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y, unit}); //top righjt
    drawBuffer();
}

void gore::imagerenderer::drawTextureRotated(GLuint texture, vec2 pos, vec2 dim, float rot){
    GLuint unit = getTextureUnit(texture);
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y, unit}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit});
    glActiveTexture(GL_TEXTURE0);
    drawBuffer();
}

void gore::imagerenderer::drawImageRotated(IMG img, vec2 pos, vec2 dim, float rot){
    GLuint unit = getTextureUnit(img->tex);
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y, unit}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit});
    drawBuffer();
}

gore::imagerenderer::imagerenderer(size_t w, size_t h) {
    allocated = 0;
    texture_unit_map.setHashFunction(hash);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    //shader.compile(std::string("img.vs"), std::string("img.fs"));
    shader.compile(vertex_shader_image, fragment_shader_image);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 5)); //rotation point
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(ivertex), (void*)(sizeof(float) * 7)); // texture unit
    shader.bind();
    matrix ortho = matrix::calculateOrtho(w, h, w, h);
    this->width = w;
    this->height = h;
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    updateView(0.0f, 0.0f, 1.0f);
}

gore::imagerenderer::imagerenderer(const imagerenderer& img) {
    this->allocated = img.allocated;
    this->texture_units = img.texture_units;
    texture_unit_map.setHashFunction(hash);
    //shader.compile(std::string("img.vs"), std::string("img.fs"));
    shader.compile(vertex_shader_image, fragment_shader_image);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 5)); //rotation point
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(ivertex), (void*)(sizeof(float) * 7)); // texture unit
    shader.bind();
    this->width = img.width;
    this->height = img.height;
    std::copy(img.vertexs.begin(), img.vertexs.end(), this->vertexs.begin());
    matrix ortho = matrix::calculateOrtho(this->width, this->height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    updateView(0.0f, 0.0f, 1.0f);
}


void gore::imagerenderer::setDimensions (uint32_t width, uint32_t height) {
    shader.bind();
    matrix ortho = matrix::calculateOrtho(width, height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    this->width = width;
    this->height = height;
}

void gore::imagerenderer::updateView (float x, float y, float zoom) {
    matrix view = matrix::calculate2DView(x, y, zoom);
    shader.setuniform("view", 1, true, view);
}

gore::grayscalerenderer::grayscalerenderer(size_t w, size_t h) : imagerenderer() {
    allocated = 0;
    shader.compile(vertex_shader_grayscale, fragment_shader_grayscale);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 5)); //rotation point
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(ivertex), (void*)(sizeof(float) * 7)); // texture unit
    shader.bind();
    matrix ortho = matrix::calculateOrtho(w, h, w, h);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    shader.setuniform("withAlpha", false);
    updateView(0.0f, 0.0f, 1.0f);
}


gore::grayscalerenderer::grayscalerenderer(const grayscalerenderer& gsr) {
    this->texture_units = gsr.texture_units;
    texture_unit_map.setHashFunction(hash);
    allocated = 0;
    shader.compile(vertex_shader_grayscale, fragment_shader_grayscale);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 5)); //rotation point
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(ivertex), (void*)(sizeof(float) * 7)); // texture unit
    shader.bind();
    this->width = gsr.width;
    this->height = gsr.height;
    std::copy(gsr.vertexs.begin(), gsr.vertexs.end(), this->vertexs.begin());
    matrix ortho = matrix::calculateOrtho(this->width, this->height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    shader.setuniform("withAlpha", false);
    updateView(0.0f, 0.0f, 1.0f);
}