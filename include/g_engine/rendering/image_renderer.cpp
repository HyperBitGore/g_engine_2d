#include "image_renderer.hpp"
#include "image_renderer_shader.hpp"
#include <memory>

std::unique_ptr<gore::image_renderer> gore::createImageRenderer (uint32_t w, uint32_t h) {
    try {
        return bindless_image_renderer::create<bindless_image_renderer>(w, h);
    } catch (render_function_not_supported& e) {
        std::cout << e.what() << " bindless texture image renderer failed construciton, trying texture unit renderer!\n";
    }
    try {
        return texture_unit_image_renderer::create<texture_unit_image_renderer>(w, h);
    } catch (render_function_not_supported& e) {
        std::cout << e.what() << " texture unit image renderer failed construciton, no more renderers to try!\n";
    }

    return nullptr;
}

void gore::bindless_image_renderer::setTextureSamplers () {
    // issue is the setting of uniform mtexture why the indexing doesn't work, fix that here
    //shader.setuniform("mtexture", samplers.size(), samplers.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_ssbo);
    auto& samplers = texture_map.getSamplers();
    glBufferData(GL_SHADER_STORAGE_BUFFER, samplers.size() * sizeof(GLuint64), samplers.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texture_ssbo);
}

void gore::bindless_image_renderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim){
    addImageVertex(texture, pos, dim, {0, 0, 1.0, 1.0}, 0.0f);
}

void gore::bindless_image_renderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim, float rot){
    addImageVertex(texture, pos, dim, {0, 0, 1.0, 1.0}, rot);
}
//first two x,y in uv is starting position in image and z, w are width and height for the uvs
void gore::bindless_image_renderer::addImageVertex(GLuint texture, vec2 pos, vec2 dim, vec4 uvs, float rot){
    uint32_t unit = static_cast<uint32_t>(texture_map.getTextureIndex(texture));
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y, unit}); //top righjt
}

void gore::bindless_image_renderer::drawBuffer() {
    assert(created && "call createRenderer before use!");
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(image_render_vertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(image_render_vertex), &vertexs[0]);
    }
    setTextureSamplers();
    glDrawArraysExt(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void gore::bindless_image_renderer::drawTexture(GLuint texture, vec2 pos, vec2 dim){
    drawTexture(texture, pos, dim, {0, 0, 1.0, 1.0});
}
void gore::bindless_image_renderer::drawTexture(GLuint texture, vec2 pos, vec2 dim, vec4 uvs){
    uint32_t unit = static_cast<uint32_t>(texture_map.getTextureIndex(texture));
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y, unit}); //top righjt
    drawBuffer();
}

void gore::bindless_image_renderer::drawImage(const IMG& img, vec2 pos, vec2 dim){
    drawImage(img, pos, dim, {0, 0, 1.0, 1.0});
}

void gore::bindless_image_renderer::drawImage(const IMG& img, vec2 pos, vec2 dim, vec4 uvs){
    uint32_t unit = static_cast<uint32_t>(texture_map.getTextureIndex(img->tex));
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y, unit}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y, unit}); //top righjt
    drawBuffer();
}

void gore::bindless_image_renderer::drawTextureRotated(GLuint texture, vec2 pos, vec2 dim, float rot){
    uint32_t unit = static_cast<uint32_t>(texture_map.getTextureIndex(texture));
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y, unit}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit});

    drawBuffer();
}

void gore::bindless_image_renderer::drawImageRotated(const IMG& img, vec2 pos, vec2 dim, float rot){
    uint32_t unit = static_cast<uint32_t>(texture_map.getTextureIndex(img->tex));
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y, unit}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y, unit}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y, unit});
    drawBuffer();
}

gore::bindless_image_renderer::bindless_image_renderer(size_t w, size_t h)
    : image_renderer(vertex_shader_image, fragment_shader_image, w, h) {
}

gore::texture_unit_image_renderer::texture_unit_image_renderer(size_t w, size_t h)
    : image_renderer(vertex_shader_texture_unit_image,
          fragment_shader_texture_unit_image, w, h) {
}

void gore::texture_unit_image_renderer::setTextureSamplers() {
    texture_map.setTextureSamplers("tex_array", shader);
}

void gore::texture_unit_image_renderer::addVertex(
    GLuint texture, vec2 pos, vec2 dim, vec4 uvs, float rot) {
    if (!texture_map.textureBinded(texture) && texture_map.full()) {
        drawBuffer();
    }
    uint32_t unit = texture_map.getTextureUnit(texture);
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y, unit});
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y, unit});
}

void gore::texture_unit_image_renderer::addImageVertex(
    GLuint texture, vec2 pos, vec2 dim) {
    addImageVertex(texture, pos, dim, {0, 0, 1.0f, 1.0f}, 0.0f);
}

void gore::texture_unit_image_renderer::addImageVertex(
    GLuint texture, vec2 pos, vec2 dim, float rot) {
    addImageVertex(texture, pos, dim, {0, 0, 1.0f, 1.0f}, rot);
}

void gore::texture_unit_image_renderer::addImageVertex(
    GLuint texture, vec2 pos, vec2 dim, vec4 uvs, float rot) {
    addVertex(texture, pos, dim, uvs, rot);
}

void gore::texture_unit_image_renderer::drawBuffer() {
    assert(created && "call createRenderer before use!");
    if (vertexs.empty()) {
        texture_map.clearUnits();
        return;
    }
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if (vertexs.size() > allocated) {
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(image_render_vertex),
            vertexs.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            vertexs.size() * sizeof(image_render_vertex), vertexs.data());
    }
    setTextureSamplers();
    glDrawArraysExt(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexs.size()));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
    texture_map.clearUnits();
}

void gore::texture_unit_image_renderer::drawImage(
    const IMG& img, vec2 pos, vec2 dim) {
    drawImage(img, pos, dim, {0, 0, 1.0f, 1.0f});
}

void gore::texture_unit_image_renderer::drawImage(
    const IMG& img, vec2 pos, vec2 dim, vec4 uvs) {
    addVertex(img->tex, pos, dim, uvs, 0.0f);
    drawBuffer();
}

void gore::texture_unit_image_renderer::drawImageRotated(
    const IMG& img, vec2 pos, vec2 dim, float rot) {
    addVertex(img->tex, pos, dim, {0, 0, 1.0f, 1.0f}, rot);
    drawBuffer();
}

void gore::texture_unit_image_renderer::drawTexture(
    GLuint texture, vec2 pos, vec2 dim) {
    drawTexture(texture, pos, dim, {0, 0, 1.0f, 1.0f});
}

void gore::texture_unit_image_renderer::drawTexture(
    GLuint texture, vec2 pos, vec2 dim, vec4 uvs) {
    addVertex(texture, pos, dim, uvs, 0.0f);
    drawBuffer();
}

void gore::texture_unit_image_renderer::drawTextureRotated(
    GLuint texture, vec2 pos, vec2 dim, float rot) {
    addVertex(texture, pos, dim, {0, 0, 1.0f, 1.0f}, rot);
    drawBuffer();
}

gore::grayscalerenderer::grayscalerenderer(size_t w, size_t h) : bindless_image_renderer(w, h) {
    allocated = 0;
    shader.compile(vertex_shader_grayscale, fragment_shader_grayscale);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(gore::image_render_vertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(gore::image_render_vertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(gore::image_render_vertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(gore::image_render_vertex), (void*)(sizeof(float) * 5)); //rotation point
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(gore::image_render_vertex), (void*)offsetof(gore::image_render_vertex, texture_unit)); // texture unit
    glGenBuffers(1, &texture_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texture_ssbo);
    shader.bind();
    matrix ortho = matrix::calculateOrtho(w, h, w, h);
    shader.setuniform("projection", 1, true, ortho);
    shader.setuniform("withAlpha", false);
}
