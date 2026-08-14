#include "three_dee_renderer.hpp"
#include "three_dee_renderer_shader.hpp"
#include "../util/gl_tagger.hpp"
#include <cstdint>
#include <stdexcept>

void gore::threedeerender::setTextureSamplers () {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_ssbo);
    auto& samplers = tm.getSamplers();
    glBufferData(GL_SHADER_STORAGE_BUFFER, samplers.size() * sizeof(GLuint64), samplers.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, texture_ssbo);
}

void gore::threedeerender::shader_setup()  {
    gl_function_tagger tags({
        "glBindVertexArray",
        "glBindBuffer",
        "glEnableVertexAttribArray",
        "glVertexAttribPointer",
        "glVertexAttribIPointer",
        "glGenBuffers",
        "glBindBufferBase",
        "glBufferData",
        "glBufferSubData",
        "glDrawElements",
        "glGetTextureHandleARB",
        "glMakeTextureHandleResidentARB",
        "glMakeTextureHandleNonResidentARB",
        "glIsTextureHandleResidentARB"
    });
    try {
        tags.hardwareSupports();
    } catch (render_function_not_supported& e) {

    }
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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glGenBuffers(1, &texture_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, texture_ssbo);
}

void gore::threedeerender::updateDimensions (uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;
    gore::matrix perspective = gore::matrix::perspective(toRadians(this->vertical_fov), (float)this->width / (float)this->height, this->near_clip, this->far_clip);
    shader.setuniform("projection", 1, true, perspective);
}

gore::threedeerender::threedeerender(uint32_t w, uint32_t h) : gore::renderer<gore::threedeerender, gore::threedee_vertex> (three_dee_renderer_vertex, three_dee_renderer_fragment, w, h) {

}

void gore::threedeerender::addModel (gore::model& model) {
    auto& ib = model.index_buffer;
    const draw_key key{reinterpret_cast<uintptr_t>(&model)};
    auto it = draw_map.find(key);
    if (it != draw_map.end()) {
        // geometry already resident, just refresh the transform and residency stamp
        it->second.last_frame = frame_count;
        model_matrices[it->second.mat_slot] = model.getMatrix();
        return;
    }
    model_matrices.push_back(model.getMatrix());
    GLint mat_slot = static_cast<GLint>(model_matrices.size()) - 1;
    GLuint base = static_cast<GLuint>(vertexs.size());
    for (auto& v : ib.getVertexs()) {
        uint32_t texture_index = 2000u;
        if (v.material_index >= 0) {
            gore::IMG& img = model.getImage(v.material_index);
            texture_index = tm.getTextureIndex(img->tex);
        }
        vertexs.push_back({v.pos.x, v.pos.y, v.pos.z, v.uv.x, v.uv.y, mat_slot, texture_index});
    }
    GLuint index_base = static_cast<GLuint>(indexs.size());
    for (GLuint i : ib.getIndexs()) {
        indexs.push_back(base + i);
    }
    instance in;
    in.index_count = static_cast<uint32_t>(ib.getIndexs().size());
    in.vertex_count = static_cast<uint32_t>(ib.getVertexs().size());
    in.index_offset = index_base;
    in.vertex_offset = base;
    in.last_frame = frame_count;
    in.mat_slot = mat_slot;
    draw_map.emplace(key, in);
    buffers_dirty = true;
}

void gore::threedeerender::addBillboard (gore::billboard& billboard, gore::camera& cam) {
    std::vector<gore::vec3> verts = billboard.getVertexs(cam);
    if (verts.size() < 6) return;

    GLuint texture_index = 2000u;
    if (billboard.img && billboard.img->tex != 0) {
        texture_index = tm.getTextureIndex(billboard.img->tex);
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
        billboard_verts[i] = {verts[i].x, verts[i].y, verts[i].z, uvs[i][0], uvs[i][1], -1, texture_index};
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

void gore::threedeerender::addTransient (const threedee_vertex* data, uint32_t count) {
    transient_vertexs.insert(transient_vertexs.end(), data, data + count);
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
    GLsizei draw_count = static_cast<GLsizei>(indexs.size() + transient_vertexs.size());
    glDrawElements(GL_TRIANGLES, draw_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    transient_vertexs.clear();
    bool evicted = false;
    for (auto it = draw_map.begin(); it != draw_map.end();) {
        if (it->second.last_frame < frame_count) {
            it = draw_map.erase(it);
            evicted = true;
        } else {
            ++it;
        }
    }
    if (evicted) {
        rebuildGeometry();
    }
    frame_count++;
}

// rebuilds resident CPU-side geometry/matrices from the surviving models
void gore::threedeerender::rebuildGeometry () {
    vertexs.clear();
    indexs.clear();
    model_matrices.clear();
    tm.clear();
    bound_textures.clear();
    std::vector<gore::model*> models;
    for (auto it = draw_map.begin(); it != draw_map.end();) {
        models.push_back(reinterpret_cast<gore::model*>(it->first.value));
        it = draw_map.erase(it);
    }
    for (gore::model* model : models) {
        addModel(*model);
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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

// uploads resident + transient geometry; only called when the draw calls changed
void gore::threedeerender::updateVertexBuffer () {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    uint32_t total = static_cast<uint32_t>(vertexs.size() + transient_vertexs.size());
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
    for (uint32_t i = 0; i < transient_vertexs.size(); i++) {
        all_indexs.push_back(static_cast<GLuint>(vertexs.size() + i));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    if (all_indexs.size() > index_allocated) {
        index_allocated = static_cast<GLuint>(all_indexs.size());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_allocated * sizeof(GLuint), all_indexs.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, all_indexs.size() * sizeof(GLuint), all_indexs.data());
    }
    buffers_dirty = false;
}

gore::instance_render::instance_render(uint32_t w, uint32_t h) : gore::renderer<gore::instance_render, gore::instance_vertex> (instance_renderer_vertex, three_dee_renderer_fragment, w, h) {

}

void gore::instance_render::updateDrawBuffers () {
    if (buffers_dirty) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(instance_vertex), vertexs.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(GLuint), indexs.data(), GL_DYNAMIC_DRAW);
        buffers_dirty = false;
    }
    if (draw_buffer_dirty) {
        glBindVertexArray(vao);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_buffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawElementsIndirectCommand), commands.data(), GL_DYNAMIC_DRAW);
        draw_buffer_dirty = false;
    }
    if (matrix_buffer_dirty) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
            matrix_array.size() * sizeof(float),
            matrix_array.data(),
            GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        matrix_buffer_dirty = false;
    }
    if (instance_texture_units_dirty) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texure_ssbo);
        std::vector<GLuint64> samplers = tm.getSamplers();
        //samplers.insert(samplers.begin(), 0); // add a dummy sampler for untextured instances
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            samplers.size() * sizeof(GLuint64),
            samplers.data(),
            GL_DYNAMIC_DRAW
        );
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, texure_ssbo);
        instance_texture_units_dirty = false;
    }
    if (texture_partition_dirty) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_index_buffer);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            texture_partition_array.size() * sizeof(uint8_t),
            texture_partition_array.getData(),
            GL_DYNAMIC_DRAW
        );
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, texture_index_buffer);
        texture_partition_dirty = false;
    }
}

void gore::instance_render::shader_setup()  {
    gl_function_tagger tags({
        "glBindVertexArray",
        "glBindBuffer",
        "glEnableVertexAttribArray",
        "glVertexAttribPointer",
        "glVertexAttribIPointer",
        "glGenBuffers",
        "glBindBufferBase",
        "glBufferData",
        "glMultiDrawElementsIndirect",
        "glGetTextureHandleARB",
        "glMakeTextureHandleResidentARB",
        "glMakeTextureHandleNonResidentARB",
        "glIsTextureHandleResidentARB"
    });
    try {
        tags.hardwareSupports();
    } catch (render_function_not_supported& e) {

    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gore::instance_vertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(gore::instance_vertex), (void*)(sizeof(float) * 3)); //uvs
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(gore::instance_vertex), (void*)(sizeof(float) * 5)); // texture unit index
    updateDimensions(this->width, this->height);
    updateView({0, 0, 5}, {0, 0, 0}, gore::vec3(0,1,0));
    shader.setuniform("set_color", {1.0f, 1.0f, 1.0f, 1.0f});
    // ssbo
    glGenBuffers(1, &ssbo);
    glGenBuffers(1, &texure_ssbo);
    glGenBuffers(1, &texture_index_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texure_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, texure_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_index_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, texture_index_buffer);
    glGenBuffers(1, &element_buffer);
    glGenBuffers(1, &draw_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_buffer);
}

// matrices
void gore::instance_render::updateDimensions (uint32_t width, uint32_t height) {
    this->width = width;
    this->height = height;
    gore::matrix perspective = gore::matrix::perspective(toRadians(this->vertical_fov), (float)this->width / (float)this->height, this->near_clip, this->far_clip);
    shader.setuniform("projection", 1, true, perspective);
}


int32_t gore::instance_render::addModelInstance (gore::model* model, const matrix& transform) {
    if (model == nullptr) return -1;
    
    auto i_in = instance_map.find(model); // indexing will create a new instance
    if (i_in == instance_map.end() || i_in->second >= instance_array.size()) {
        return -1;
    }
    auto& it = instance_array[i_in->second];
    auto& call = commands[it.command];
    // add an instance and matrix
    // this needs to align with the instance location
    int index = addModelMatrix(model, transform);
    if (index < 0) {
        return -1;
    }
    // model_matrices.insert(model_matrices.begin() + call.base_instance + call.instance_count, transform);
    call.instance_count++;
    draw_buffer_dirty = true;
    matrix_buffer_dirty = true;
    return index;
}

void gore::instance_render::addModelData(gore::model* model, uint32_t preallocate) {
    if (model == nullptr || instance_map.find(model) != instance_map.end()) return;
    // add a new draw_command and model gets inserted
    DrawElementsIndirectCommand command;
    auto& ib = model->index_buffer;
    // base_instance is the model's fixed offset into the matrix / texture-unit SSBOs
    command.base_instance = (GLuint)current_matrix_size;
    command.instance_count = 0;
    command.base_vertex = 0;
    command.count = static_cast<GLuint>(model->index_buffer.indexSize());
    command.first_index = static_cast<GLuint>(indexs.size());
    uint32_t tex_index = UINT32_MAX;
    if (!model->getImages().empty()) {
        gore::IMG& img = model->getImages()[0];
        tex_index = tm.getTextureIndex(img->tex);
    }
    instance in = {
        static_cast<int32_t>(commands.size()),
        command.first_index,
        command.count,
        static_cast<uint32_t>(vertexs.size()),
        static_cast<uint32_t>(model->index_buffer.vertexSize()),
        current_matrix_size,
        preallocate,
        0,
        tex_index
    };
    instance_map.emplace(model, static_cast<uint32_t>(instance_array.size()));
    instance_array.push_back(in);
    commands.push_back(command);
    current_matrix_size += preallocate;
    texture_partition_array.addPartition(reinterpret_cast<uintptr_t>(model), sizeof(GLuint) * preallocate);
    preallocateMatrixArray(model);
    // add model data
    uint32_t base = static_cast<uint32_t>(vertexs.size());
    for (auto& v : ib.getVertexs()) {
        uint32_t texture_unit = 2000u;
        vertexs.push_back({v.pos.x, v.pos.y, v.pos.z, v.uv.x, v.uv.y});
    }
    GLuint index_base = static_cast<GLuint>(indexs.size());
    for (GLuint i : ib.getIndexs()) {
        indexs.push_back(base + i);
    }
    buffers_dirty = true;
    matrix_buffer_dirty = true;
    draw_buffer_dirty = true;
    texture_partition_dirty = true;
    instance_texture_units_dirty = true;
}
void gore::instance_render::removeModelInstance (gore::model* model, int32_t index) {
    auto i_in = instance_map.find(model);
     if (i_in == instance_map.end() || i_in->second >= instance_array.size()) {
        return;
    }
    auto& it = instance_array[i_in->second];
    auto& call =  commands[it.command];
    int m_index = removeModelMatrix(model, index);
    if (m_index < 0) {
        return;
    }
    texture_partition_array.removePartitionData(reinterpret_cast<uintptr_t>(model), m_index * sizeof(GLuint), sizeof(GLuint));
    call.instance_count--;
    draw_buffer_dirty = true;
    matrix_buffer_dirty = true;
    instance_texture_units_dirty = true;
    texture_partition_dirty = true;
}

void gore::instance_render::updateModelInstance (gore::model* model, int32_t index, const matrix& transform) {
    auto it = instance_map.find(model);
    if (it == instance_map.end()) {
        return;
    }
    auto& in = instance_array[it->second];
    if (index >= 0 && index < in.current_matrix_index) {
        memcpy(matrix_array.data() + ( in.matrix_offset + index ) * 16, const_cast<matrix&>(transform).data(), 16 * sizeof(float));
        matrix_buffer_dirty = true;
    }
}

void gore::instance_render::preallocateMatrixArray (model* model) {
    auto it = instance_map.find(model);
    if (it != instance_map.end()) {
        auto& in = instance_array[it->second];
        if (matrix_array.empty()) {
            matrix_array = std::vector(in.matrix_size * 16, 0.0f);
            return;
        }
        matrix_array.resize(matrix_array.size() + (in.matrix_size * 16), 0.0f);
    }
}

// issue is second models data not exisiting when we allocate
// bad alloc only caught later
// only double the target region
void gore::instance_render::reallocateMatrixArray (model* model) {
    auto it = instance_map.find(model);
    uint32_t new_size = 0;
    for (uint32_t i = 0; i < instance_array.size(); i++) {
        if (i == it->second) {
            new_size += ( instance_array[i].matrix_size * 16 ) * 2;
        } else {
            new_size += ( instance_array[i].matrix_size * 16 );
        }
    }
    // reallocate and copy the matrices forward
    std::vector<float> new_array(new_size, 0.0f);
    uint32_t new_offset = 0; // in matrix slots
    for (uint32_t i = 0; i < instance_array.size(); i++) {
        auto& in = instance_array[i];
        std::copy(matrix_array.begin() + (in.matrix_offset * 16), 
        matrix_array.begin() + ((in.matrix_offset + in.current_matrix_index) * 16), 
        new_array.begin() + (new_offset * 16));
        in.matrix_offset = new_offset;
        if (i == it->second) {
            // keep the texture-unit partition the same capacity as the matrix region
            texture_partition_array.extendPartition(reinterpret_cast<uintptr_t>(model), in.matrix_size * sizeof(GLuint));
            in.matrix_size *= 2;
        }
        commands[in.command].base_instance = (GLuint)new_offset;
        new_offset += in.matrix_size;
    }
    matrix_array = std::move(new_array);
    matrix_buffer_dirty = true;
    draw_buffer_dirty = true;
    texture_partition_dirty = true;
}

int32_t gore::instance_render::addModelMatrix (model* model, const matrix& transform) {
    auto it = instance_map.find(model);
    if (it == instance_map.end()) {
        return -1;
    }
    auto& in = instance_array[it->second];
    if (in.current_matrix_index >= in.matrix_size) {
        reallocateMatrixArray(model);
    }
    memcpy(matrix_array.data() + (in.matrix_offset + in.current_matrix_index ) * 16, const_cast<matrix&>(transform).data(), 16 * sizeof(float));
    matrix_buffer_dirty = true;
    // issue here, switch to same manner of management as matrix array, maybe do a class for fixed size array with dynamic growth, or just use std::vector and manage the offsets
    texture_partition_array.addPartitionData(reinterpret_cast<uintptr_t>(model), reinterpret_cast<const uint8_t*>(&in.tex_unit), sizeof(GLuint));
    texture_partition_dirty = true;
    in.current_matrix_index++;
    return in.current_matrix_index - 1;
}
// this has issues,
// also fix the 0.01 matrix getting copied into the wrong model space btw
int32_t gore::instance_render::removeModelMatrix (model* model, int32_t index) {
    auto it = instance_map.find(model);
    if (it == instance_map.end() || index < 0) {
        return -1;
    }
    auto& in = instance_array[it->second];
    // index is relative to this model's instances
    if (index >= in.current_matrix_index) {
        return -1;
    }
    const uint32_t remaining = in.current_matrix_index - index - 1;
    if (remaining > 0) {
        std::memmove(
            matrix_array.data() + (in.matrix_offset + index) * 16,
            matrix_array.data() + (in.matrix_offset + index + 1) * 16,
            remaining * 16 * sizeof(float));
    }
    in.current_matrix_index -= 1;
    return index;
}

void gore::instance_render::drawBuffer() {
    assert(created && "call createRenderer before use!");
    if (vertexs.empty() || commands.empty()) return;
    shader.bind();
    updateDrawBuffers();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, texure_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, texture_index_buffer);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_buffer);
    glMultiDrawElementsIndirect(GL_TRIANGLES,  GL_UNSIGNED_INT, (void*)(0), static_cast<GLsizei>(commands.size()), 0);
    glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}