#include "g_engine_2d.h"

void PrimitiveRenderer::setColor(vec4 color){
    triangle_shader.setuniform("set_color", color);
    point_shader.setuniform("set_color", color);
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
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
}
void PrimitiveRenderer::drawBufferTriangle(){
    triangle_shader.bind();
    glBindVertexArray_g(triangle_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
}
//quads
void PrimitiveRenderer::addQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});  
}
void PrimitiveRenderer::drawQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});  
    triangle_shader.bind();
    glBindVertexArray_g(triangle_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
}
void PrimitiveRenderer::drawBufferQuad(){
    triangle_shader.bind();
    glBindVertexArray_g(triangle_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
}
//points
void PrimitiveRenderer::addPoint(vec2 p){
    vertexs.push_back(p);
}
void PrimitiveRenderer::drawPoint(vec2 p){
    point_shader.bind();
    vertexs.push_back(p);
    glBindVertexArray_g(triangle_vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData_g(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArrays_g(GL_POINTS, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray_g(0);
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
    //bind buffers per vertexatrribarray,if you don't bind buffers, attribarrays break
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray_g(0);
    glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    
    point_shader.compile(std::string("point.vs"), std::string("point.fs"));
    point_shader.bind();
    point_shader.setuniform("point_size", 1.0f);
    point_shader.setuniform("screen", {(float)sw, (float)sh});
}