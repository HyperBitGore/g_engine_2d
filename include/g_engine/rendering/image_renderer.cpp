#include "image_renderer.hpp"
#include "image_renderer_shader.hpp"

void gore::imagerenderer::addImageVertex(vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
}

void gore::imagerenderer::addImageVertex(vec2 pos, vec2 dim, float rot){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
}
//first two x,y in uv is starting position in image and z, w are width and height for the uvs
void gore::imagerenderer::addImageVertex(vec2 pos, vec2 dim, vec4 uvs, float rot){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, rot, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y}); //top righjt
}

void gore::imagerenderer::drawBuffer(IMG img){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
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
}

void gore::imagerenderer::drawBuffer(GLuint texture){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
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
}

void gore::imagerenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
    drawBuffer(texture);
}
void gore::imagerenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim, vec4 uvs){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y}); //top righjt
    drawBuffer(texture);
}

void gore::imagerenderer::drawImage(IMG img, vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
    drawBuffer(img);
}

void gore::imagerenderer::drawImage(IMG img, vec2 pos, vec2 dim, vec4 uvs){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y}); //top righjt
    drawBuffer(img);
}

void gore::imagerenderer::drawTextureRotated(GLuint texture, vec2 pos, vec2 dim, float rot){
      vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
    glActiveTexture(GL_TEXTURE0);
    drawBuffer(texture);
}

void gore::imagerenderer::drawImageRotated(IMG img, vec2 pos, vec2 dim, float rot){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
    drawBuffer(img);
}

gore::imagerenderer::imagerenderer(size_t w, size_t h) {
    allocated = 0;
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
    shader.bind();
    matrix ortho = matrix::calculateOrtho(w, h, w, h);
    this->width = w;
    this->height = h;
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
}

gore::imagerenderer::imagerenderer(const imagerenderer& img) {
    this->allocated = img.allocated;
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
    shader.bind();
    this->width = img.width;
    this->height = img.height;
    std::copy(img.vertexs.begin(), img.vertexs.end(), this->vertexs.begin());
    matrix ortho = matrix::calculateOrtho(this->width, this->height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
}


void gore::imagerenderer::setDimensions (uint32_t width, uint32_t height) {
    shader.bind();
    matrix ortho = matrix::calculateOrtho(width, height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    this->width = width;
    this->height = height;
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
    shader.bind();
    matrix ortho = matrix::calculateOrtho(w, h, w, h);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    shader.setuniform("withAlpha", false);
}


gore::grayscalerenderer::grayscalerenderer(const grayscalerenderer& gsr) {
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
    shader.bind();
    this->width = gsr.width;
    this->height = gsr.height;
    std::copy(gsr.vertexs.begin(), gsr.vertexs.end(), this->vertexs.begin());
    matrix ortho = matrix::calculateOrtho(this->width, this->height, this->width, this->height);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("mtexture", (GLuint)0);
    shader.setuniform("withAlpha", false);
}