#include "wireframe_renderer.hpp"
#include "wireframe_renderer_shader.hpp"

void gore::wireframe_renderer::shader_setup() {
    setColor({1.0f, 1.0f, 1.0f, 1.0f});
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gore::wireframe_vertex), (void*)0); //position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(gore::wireframe_vertex), (void*)(sizeof(float) * 3)); //rotation
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(gore::wireframe_vertex), (void*)(sizeof(float) * 4)); //rotation point
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(gore::wireframe_vertex), (void*)(sizeof(float) * 7)); //axis
}

gore::wireframe_renderer::wireframe_renderer(uint32_t width, uint32_t height) : renderer<gore::wireframe_renderer, gore::wireframe_vertex> (wireframe_vertex_shader, wireframe_fragment_shader, width, height) {

}

inline GLuint axisToUint (gore::axis axis) {
    switch (axis) {
        case gore::axis::x:
        return 0;
        case gore::axis::y:
        return 1;
        case gore::axis::z:
        return 2;
        default:
        return 3;
    }
}

void gore::wireframe_renderer::addVertex(gore::vec3 p, float rotation, axis axis, gore::vec3 rotation_point) {
    vertexs.push_back({p.x, p.y, p.z, rotation, rotation_point.x, rotation_point.y, rotation_point.z, axisToUint(axis)});
}

void gore::wireframe_renderer::addVertex (gore::vec3 p) {
    addVertex(p, 0.0f, gore::axis::nil, p);
}

void gore::wireframe_renderer::addLine (gore::vec3 p1, gore::vec3 p2, float rotation, gore::axis axis, gore::vec3 rotation_point) {
    addVertex(p1, rotation, axis, rotation_point);
    addVertex(p2, rotation, axis, rotation_point);
}

void gore::wireframe_renderer::addLine (gore::vec3 p1, gore::vec3 p2) {
    addVertex(p1);
    addVertex(p2);
}
void gore::wireframe_renderer::addTriangle (gore::vec3 p1, gore::vec3 p2, gore::vec3 p3, float rotation, gore::axis axis, gore::vec3 rotation_point) {
    // line 1
    addLine(p1, p2, rotation, axis, rotation_point);
    // line 2
    addLine(p2, p3, rotation, axis, rotation_point);
    // line 3
    addLine(p3, p1, rotation, axis, rotation_point);
}

// make sure they are a proper triangle lol
void gore::wireframe_renderer::addTriangle (gore::vec3 p1, gore::vec3 p2, gore::vec3 p3) {
    // line 1
    addLine(p1, p2);
    // line 2
    addLine(p2, p3);
    // line 3
    addLine(p3, p1);
}

void gore::wireframe_renderer::drawBuffer() {
    assert(created && "call createRenderer before use!");
    glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    shader.bind();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(wireframe_vertex), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(wireframe_vertex), &vertexs[0]);
    }
    glDrawArraysExt(GL_LINES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
    glDisable(GL_LINE_SMOOTH);
}
void gore::wireframe_renderer::setColor (gore::vec4 color) {
    shader.setuniform("set_color", color);
}