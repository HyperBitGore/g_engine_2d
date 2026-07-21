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
        bound_textures.push_back(texture);
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
    // rebind textures in case another renderer clobbered the units between frames
    for (size_t i = 0; i < samplers.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + samplers[i]);
        glBindTexture(GL_TEXTURE_2D, bound_textures[i]);
    }
    shader.setuniform("textures", samplers.size(), samplers.data());
}

void gore::threedeerender::shader_setup()  {
    texture_unit_map.setHashFunction(hash);
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
        // out of texture units: flush and drop all resident geometry so units can be reassigned
        drawBuffer();
        model_map.clear();
        vertexs.clear();
        indexs.clear();
        model_matrices.clear();
        texture_unit_map.clear();
        samplers.clear();
        bound_textures.clear();
        current_unit = 0;
        // billboard texture units are invalid after a reset, drop cached transients too
        transient_map.clear();
        transient_vertexs.clear();
        buffers_dirty = true;
    }
    auto it = model_map.find(&model);
    if (it != model_map.end()) {
        // geometry already resident, just refresh the transform and residency stamp
        it->second.last_frame = frame_count;
        model_matrices[it->second.mat_slot] = model.getMatrix();
        return;
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
    GLuint index_base = indexs.size();
    for (GLuint i : ib.getIndexs()) {
        indexs.push_back(base + i);
    }
    instance in;
    in.index_count = ib.getIndexs().size();
    in.vertex_count = ib.getVertexs().size();
    in.index_offset = index_base;
    in.vertex_offset = base;
    in.last_frame = frame_count;
    in.mat_slot = mat_slot;
    model_map[&model] = in;
    buffers_dirty = true;
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
    threedee_vertex billboard_verts[6];
    for (int i = 0; i < 6; i++) {
        billboard_verts[i] = {verts[i].x, verts[i].y, verts[i].z, uvs[i][0], uvs[i][1], -1, texture_unit};
    }
    addTransient(billboard_verts, 6);
}

// unskinned vertex
void gore::threedeerender::addTriangle(gore::vec3 pos, gore::vec3 pos2, gore::vec3 pos3) {
    threedee_vertex tri[3] = {
        {pos.x, pos.y, pos.z, 0.0, 0.0, -1, 2000u},
        {pos2.x, pos2.y, pos2.z, 0.0, 0.0, -1, 2000u},
        {pos3.x, pos3.y, pos3.z, 0.0, 0.0, -1, 2000u},
    };
    addTransient(tri, 3);
}
// unskinned vertexs
void gore::threedeerender::addVertexs(const std::vector<gore::vec3>& vertexs) {
    std::vector<threedee_vertex> vs;
    vs.reserve(vertexs.size());
    for (auto& i : vertexs) {
        vs.push_back({i.x, i.y, i.z, 0.0, 0.0, -1, 2000u});
    }
    addTransient(vs.data(), vs.size());
}

size_t gore::threedeerender::hashVertexs (const threedee_vertex* data, size_t count) {
    // FNV-1a over the raw vertex bytes
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
    size_t n = count * sizeof(threedee_vertex);
    size_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) {
        h ^= bytes[i];
        h *= 1099511628211ull;
    }
    return h;
}

void gore::threedeerender::addTransient (const threedee_vertex* data, size_t count) {
    size_t key = hashVertexs(data, count);
    auto it = transient_map.find(key);
    if (it != transient_map.end()) {
        // identical call already resident, just refresh residency stamp
        it->second.last_frame = frame_count;
        return;
    }
    transient_entry e;
    e.verts.assign(data, data + count);
    e.offset = transient_vertexs.size();
    e.last_frame = frame_count;
    transient_vertexs.insert(transient_vertexs.end(), data, data + count);
    transient_map.emplace(key, std::move(e));
    buffers_dirty = true;
}

// repacks transient_vertexs from surviving map entries
void gore::threedeerender::rebuildTransients () {
    transient_vertexs.clear();
    for (auto& entry : transient_map) {
        entry.second.offset = transient_vertexs.size();
        transient_vertexs.insert(transient_vertexs.end(), entry.second.verts.begin(), entry.second.verts.end());
    }
    buffers_dirty = true;
}

void gore::threedeerender::drawBuffer() {
    assert(created && "call createRenderer before use!");
    if (vertexs.empty() && transient_vertexs.empty()) return;
    shader.bind();
    setTextureSamplers();
    if (buffers_dirty) {
        updateVertexBuffer();
    }
    uploadMatrices(); // transforms can change every frame, always upload
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    GLsizei draw_count = (GLsizei)(indexs.size() + transient_vertexs.size());
    glDrawElements(GL_TRIANGLES, draw_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // evict models not submitted this frame
    bool evicted = false;
    for (auto it = model_map.begin(); it != model_map.end();) {
        if (it->second.last_frame < frame_count) { it = model_map.erase(it); evicted = true; }
        else { ++it; }
    }
    if (evicted) {
        rebuildGeometry();
    }
    // evict transient calls not submitted this frame
    bool transient_evicted = false;
    for (auto it = transient_map.begin(); it != transient_map.end();) {
        if (it->second.last_frame < frame_count) { it = transient_map.erase(it); transient_evicted = true; }
        else { ++it; }
    }
    if (transient_evicted) {
        rebuildTransients();
    }
    frame_count++;
}

// rebuilds resident CPU-side geometry/matrices from the surviving models
void gore::threedeerender::rebuildGeometry () {
    vertexs.clear();
    indexs.clear();
    model_matrices.clear();
    texture_unit_map.clear();
    samplers.clear();
    bound_textures.clear();
    current_unit = 0;
    auto temp_map = model_map;
    model_map.clear();
    for (auto& entry : temp_map) {
        addModel(*(entry.first));
    }
    buffers_dirty = true;
}

void gore::threedeerender::uploadMatrices () {
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
}

// uploads resident + transient geometry; only called when the draw calls changed
void gore::threedeerender::updateVertexBuffer () {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    size_t total = vertexs.size() + transient_vertexs.size();
    if (total > allocated) {
        allocated = total;
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(threedee_vertex), nullptr, GL_DYNAMIC_DRAW);
    }
    if (!vertexs.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(threedee_vertex), vertexs.data());
    }
    if (!transient_vertexs.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(threedee_vertex), transient_vertexs.size() * sizeof(threedee_vertex), transient_vertexs.data());
    }
    // element buffer: resident indices followed by sequential indices for transient vertexs
    std::vector<GLuint> all_indexs = indexs;
    all_indexs.reserve(indexs.size() + transient_vertexs.size());
    for (size_t i = 0; i < transient_vertexs.size(); i++) {
        all_indexs.push_back((GLuint)(vertexs.size() + i));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    if (all_indexs.size() > index_allocated) {
        index_allocated = all_indexs.size();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_allocated * sizeof(GLuint), all_indexs.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, all_indexs.size() * sizeof(GLuint), all_indexs.data());
    }
    buffers_dirty = false;
}