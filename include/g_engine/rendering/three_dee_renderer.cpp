#include "three_dee_renderer.hpp"
#include "three_dee_renderer_shader.hpp"

GLuint gore::threedeerender::getTextureUnit (GLuint texture) {
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

bool gore::threedeerender::textureBinded (GLuint texture) {
    GLuint* unit = texture_unit_map.get(texture);
    return unit != nullptr;
}

void gore::threedeerender::setTextureSamplers () {
    shader.setuniform("textures", samplers.size(), samplers.data());
}

void gore::threedeerender::shader_setup()  {
    texture_unit_map.setHashFunction(hash);
    model_map.setHashFunction(hash_model);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gore::threedee_vertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 3)); //uvs
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 5)); // model matrice index
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(gore::threedee_vertex), (void*)(sizeof(float) * 6)); // texture unit index
    updateDimensions(this->width, this->height);
    updateView({0, 0, 5}, {0, 0, 0}, gore::vec3(0,1,0));
    shader.setuniform("set_color", {1.0f, 1.0f, 1.0f, 1.0f});
    // ssbo
    glGenBuffers(1, &ssbo);
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
}

void gore::threedeerender::updateDimensions (uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;
    gore::matrix perspective = gore::matrix::perspective(toRadians(this->vertical_fov), (float)this->width / (float)this->height, this->near_clip, this->far_clip);
    shader.setuniform("projection", 1, true, perspective);
}

gore::threedeerender::threedeerender(size_t w, size_t h) : gore::renderer<gore::threedeerender, gore::threedee_vertex> (three_dee_renderer_vertex, three_dee_renderer_fragment, w, h) {

}

void gore::threedeerender::addModel (gore::model& model) {
    auto& ib = model.index_buffer;
    uint32_t needed = model.textureCount();
    if (current_unit + needed > (uint32_t)texture_units) {
        drawBuffer();
    }
    model_matrices.push_back(model.getMatrix());
    GLint mat_slot = (GLint)model_matrices.size() - 1;
    GLuint base = (GLuint)vertexs.size();
    for (auto& v : ib.getVertexs()) {
        uint32_t texture_unit = 2000u;
        if (v.material_index >= 0) {
            gore::IMG& img = model.getImage(v.material_index);
            texture_unit = getTextureUnit(img->tex);
        }
        vertexs.push_back({v.pos.x, v.pos.y, v.pos.z, v.uv.x, v.uv.y, mat_slot, texture_unit});
    }
    for (GLuint i : ib.getIndexs()) {
        indexs.push_back(base + i);
    }
}

void gore::threedeerender::addModelInstance (gore::model* model, const matrix& transform) {
    
}

void gore::threedeerender::addBillboard (gore::billboard& billboard, gore::camera& cam) {
    std::vector<gore::vec3> verts = billboard.getVertexs(cam);
    if (verts.size() < 6) return;

    GLuint texture_unit = 2000u;
    if (billboard.img && billboard.img->tex != 0) {
        if (!textureBinded(billboard.img->tex) && current_unit == texture_units) {
            drawBuffer();
        }
        texture_unit = getTextureUnit(billboard.img->tex);
    }

    // UV layout matches getVertexs triangle order: tl, bl, br, tl, br, tr
    const float uvs[6][2] = {
        {0.0f, 1.0f}, // tl
        {0.0f, 0.0f}, // bl
        {1.0f, 0.0f}, // br
        {0.0f, 1.0f}, // tl
        {1.0f, 0.0f}, // br
        {1.0f, 1.0f}, // tr
    };
    for (int i = 0; i < 6; i++) {
        indexs.push_back((GLuint)vertexs.size());
        vertexs.push_back({verts[i].x, verts[i].y, verts[i].z, uvs[i][0], uvs[i][1], -1, texture_unit});
    }
}

// unskinned vertex
void gore::threedeerender::addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3) {
    indexs.push_back((GLuint)vertexs.size());
    vertexs.push_back({pos.x, pos.y, pos.z, 0.0, 0.0, -1, 2000u});
    indexs.push_back((GLuint)vertexs.size());
    vertexs.push_back({pos2.x, pos2.y, pos2.z, 0.0, 0.0, -1, 2000u});
    indexs.push_back((GLuint)vertexs.size());
    vertexs.push_back({pos3.x, pos3.y, pos3.z, 0.0, 0.0, -1, 2000u});
}
// unskinned vertexs
void gore::threedeerender::addVertexs(const std::vector<gore::vec3>& vertexs) {
    for (auto& i : vertexs) {
        this->indexs.push_back((GLuint)this->vertexs.size());
        this->vertexs.push_back({i.x, i.y, i.z, 0.0, 0.0, -1, 2000u});
    }
}

void gore::threedeerender::drawBuffer() {
    assert(created && "call createRenderer before use!");
    if (vertexs.empty()) return;
    shader.bind();
    setTextureSamplers();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(threedee_vertex), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(threedee_vertex), &vertexs[0]);
    }
    std::vector<float> flat_matrices;
    flat_matrices.reserve(model_matrices.size() * 16);
    for (auto& m : model_matrices) {
        float* d = m.data();
        flat_matrices.insert(flat_matrices.end(), d, d + 16);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        flat_matrices.size() * sizeof(float),
        flat_matrices.data(),
        GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    if (indexs.size() > index_allocated) {
        index_allocated = indexs.size();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_allocated * sizeof(GLuint), indexs.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexs.size() * sizeof(GLuint), indexs.data());
    }
    glDrawElements(GL_TRIANGLES, (GLsizei)indexs.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
    indexs.clear();
    texture_unit_map.clear();
    samplers.clear();
    current_unit = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    model_matrices.clear();
}