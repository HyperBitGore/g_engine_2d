#include "g_engine_2d.h"

void PrimitiveRenderer::setColor(vec4 color){
    triangle_shader.setuniform("set_color", vec4(255, 0, 0, 0));
}

//triangles
void PrimitiveRenderer::addTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
}
void PrimitiveRenderer::drawTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
    triangle_shader.bind();
    glBindVertexArray_g(triangle_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() >= allocated){
        allocated = allocated << 1;
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
    glBindBuffer_g(GL_ARRAY_BUFFER, 0);
}
void PrimitiveRenderer::drawBufferTriangle(){
    triangle_shader.bind();
    if(vertexs.size() > allocated){
        allocated = allocated << 1;
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
}
//quads
void PrimitiveRenderer::addQuad(vec2 pos, float w, float h){

}
void PrimitiveRenderer::drawQuad(vec2 pos, float w, float h){

}
void PrimitiveRenderer::drawBufferQuad(){

}
//points
void PrimitiveRenderer::addPoint(vec2 p){

}
void PrimitiveRenderer::drawPoint(vec2 p){

}
void PrimitiveRenderer::drawBufferPoint(){

}
//lines
void PrimitiveRenderer::addLine(vec2 p1, vec2 p2){

}
void PrimitiveRenderer::drawLine(vec2 p1, vec2 p2){

}
void PrimitiveRenderer::drawBufferLine(){

}

PrimitiveRenderer::PrimitiveRenderer(GLuint sw, GLuint sh) {
    vertexs.reserve(1000);
    allocated = 1;
    glGenBuffers_g(1, &vertex_buffer);
    triangle_shader.compile(std::string("trian.vs"), std::string("trian.fs"));
    triangle_shader.bind();
    triangle_shader.setuniform("screen", {(float)sw, (float)sh});
    glGenVertexArrays_g(1, &triangle_vao);
    glBindVertexArray_g(triangle_vao);
    glEnableVertexAttribArray_g(0);
    glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    //glBufferData_g(GL_ARRAY_BUFFER, 1000 * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW); //only call this again when we go over the allocated data else we will use glBufferSubData instead
}