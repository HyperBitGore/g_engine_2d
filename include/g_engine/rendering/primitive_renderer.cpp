#include "primitive_renderer.hpp"
#include "../util/matrix.hpp"
#include "primitive_renderer_shader.hpp"

void gore::primitiverenderer::setColor(vec4 color){
    triangle_shader.setuniform("set_color", color);
    point_shader.setuniform("set_color", color);
    line_shader.setuniform("set_color", color);
}

//triangles
void gore::primitiverenderer::addTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
}
void gore::primitiverenderer::drawTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
    triangle_shader.bind();
    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
void gore::primitiverenderer::drawBufferTriangle(){
    triangle_shader.bind();
    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
//quads
void gore::primitiverenderer::addQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});  
}
void gore::primitiverenderer::drawQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});  
    triangle_shader.bind();
    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
void gore::primitiverenderer::drawBufferQuad(){
    triangle_shader.bind();
    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_TRIANGLES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
//points
void gore::primitiverenderer::addPoint(vec2 p){
    vertexs.push_back(p);
}
void gore::primitiverenderer::drawPoint(vec2 p){
    point_shader.bind();
    vertexs.push_back(p);
    glBindVertexArray(point_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_POINTS, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
void gore::primitiverenderer::drawBufferPoint(){
    point_shader.bind();
    glBindVertexArray(point_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_POINTS, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
}
//lines
void gore::primitiverenderer::addLine(vec2 p1, vec2 p2){
    vertexs.push_back(p1);
    vertexs.push_back(p2);
}
void gore::primitiverenderer::drawLine(vec2 p1, vec2 p2){
    glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    vertexs.push_back(p1);
    vertexs.push_back(p2);
    line_shader.bind();
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_LINES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
   glDisable(GL_LINE_SMOOTH);
}
void gore::primitiverenderer::drawBufferLine(){
    glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    line_shader.bind();
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
        glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(vec2), &vertexs[0], GL_DYNAMIC_DRAW);
    }else{
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(vec2), &vertexs[0]);
    }
    glDrawArraysExt(GL_LINES, 0, (GLsizei)vertexs.size());
    vertexs.clear();
    glBindVertexArray(0);
   glDisable(GL_LINE_SMOOTH);
}

void gore::primitiverenderer::setLineWidth(float l){
    glLineWidth(l);
}

void gore::primitiverenderer::addLinearBezier(vec2 p1, vec2 p2){
    int subdiv = (int)std::abs(p1.x - p2.x) + (int)std::abs(p1.y - p2.y);
	float step = 1.0f / (float)subdiv;
	//passing point or not reaching it, need to re-examine how many subdiv to do
	for (int i = 0; i < subdiv; i++) {
		float t = i * step;
		float x = (1 - t) * p1.x + t * p2.x;
		float y = (1 - t) * p1.y + t * p2.y;
		vertexs.push_back({ x, y });
	}
}

void gore::primitiverenderer::drawLinearBezier(vec2 p1, vec2 p2) {
    addLinearBezier(p1, p2);
    drawBufferLine();
}

void gore::primitiverenderer::addQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv){
    float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * p1.x + 2 * t1 * t * p2.x + t2 * p3.x;
		float y = t1 * t1 * p1.y + 2 * t1 * t * p2.y + t2 * p3.y;
		(i == 0) ? lx = x, ly = y : lx = lx, ly = ly;
		vertexs.push_back({ lx, ly });
		vertexs.push_back({ x, y });
		lx = x;
		ly = y;
	}
}

void gore::primitiverenderer::drawQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv) {
    addQuadraticBezier(p1, p2, p3, subdiv);
    drawBufferLine();
}

void gore::primitiverenderer::addCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv){
    float step = 1.0f / subdiv;
	float lx = 0, ly = 0;
	for (int i = 0; i <= subdiv; i++) {
		float t = i * step;
		float t1 = (1.0f - t);
		float t2 = t * t;
		float x = t1 * t1 * t1 * p1.x + 3 * t1 * t1 * t * p2.x + 3 * t1 * t2 * p3.x + t2 * t * p4.x;
		float y = t1 * t1 * t1 * p1.y + 3 * t1 * t1 * t * p2.y + 3 * t1 * t2 * p3.y + t2 * t * p4.y;
		(i == 0) ? lx = x, ly = y : lx = lx, ly = ly;
		vertexs.push_back({ lx, ly });
		vertexs.push_back({ x, y });
		lx = x;
		ly = y;
	}
}
void gore::primitiverenderer::drawCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv) {
    addCubicBezier(p1, p2, p3, p4, subdiv);
    drawBufferLine();
}
void gore::primitiverenderer::addCircleFilled(vec2 p, float r){
    float step = 2.0f * M_PI / 720.0f;
    for (float ang = 0; ang < 2.0f * M_PI; ang += step) {
        vertexs.push_back(p);
        vertexs.push_back({p.x + r * cosf(ang), p.y + r * sinf(ang)});
        vertexs.push_back({p.x + r * cosf(ang + step), p.y + r * sinf(ang + step)});
    }
}

void gore::primitiverenderer::drawCircleFilled(vec2 p, float r){
    addCircleFilled(p, r);
    drawBufferTriangle();
}

void gore::primitiverenderer::addCircleOutline(vec2 p, float r, uint32_t segments) {
    float step = 2.0f * M_PI / segments;
    for (size_t i = 0; i < segments; i++) {
        float a0 = i * (float)step;
        float a1 = (i + 1) * (float)step;
        addLine({p.x + r * std::cosf(a0), p.y + r * sinf(a0)}, {p.x + r * std::cosf(a1), p.y + r * std::sinf(a1)});
    }
}

void gore::primitiverenderer::drawCircleOutline(vec2 p, float r, uint32_t segments) {
    addCircleOutline(p, r, segments);
    drawBufferLine();
}

gore::primitiverenderer::primitiverenderer(GLuint sw, GLuint sh) {
    matrix ortho = matrix::calculateOrtho(sw, sh, sw, sh);
    vertexs.reserve(1000);
    allocated = 1;
    glGenBuffers(1, &vertex_buffer);
    triangle_shader.compile(triangle_vertex, triangle_fragment);
    triangle_shader.bind();
    triangle_shader.setuniform("projection", 1, true, ortho);
    glGenVertexArrays(1, &triangle_vao);
    glBindVertexArray(triangle_vao);
    //bind buffers per vertexatrribarray,if you don't bind buffers, attribarrays break
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    
    point_shader.compile(point_vertex, point_fragment);
    point_shader.bind();
    glGenVertexArrays(1, &point_vao);
    glBindVertexArray(point_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    point_shader.setuniform("point_size", 1.0f);
    point_shader.setuniform("projection", 1, true, ortho);

    line_shader.compile(line_vertex, line_fragment);
    line_shader.bind();
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    line_shader.setuniform("projection", 1, true, ortho);
    setLineWidth(1.0f);
    this->width = sw;
    this->height = sh;
    updateView(0.0f, 0.0f, 1.0f);
}
gore::primitiverenderer::primitiverenderer(const primitiverenderer& p) {
    this->width = p.width;
    this->height = p.height;
    matrix ortho = matrix::calculateOrtho(this->width, this->height, this->width, this->height);
    vertexs.reserve(1000);
    std::copy(p.vertexs.begin(), p.vertexs.end(), vertexs.begin());
    this->allocated = p.allocated;
    glGenBuffers(1, &vertex_buffer);
    triangle_shader.compile(triangle_vertex, triangle_fragment);
    triangle_shader.bind();
    triangle_shader.setuniform("projection", 1, true, ortho);
    glGenVertexArrays(1, &triangle_vao);
    glBindVertexArray(triangle_vao);
    //bind buffers per vertexatrribarray,if you don't bind buffers, attribarrays break
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    
    point_shader.compile(point_vertex, point_fragment);
    point_shader.bind();
    glGenVertexArrays(1, &point_vao);
    glBindVertexArray(point_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    point_shader.setuniform("point_size", 1.0f);
    point_shader.setuniform("projection", 1, true, ortho);

    line_shader.compile(line_vertex, line_fragment);
    line_shader.bind();
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    line_shader.setuniform("projection", 1, true, ortho);
    setLineWidth(1.0f);
}

void gore::primitiverenderer::setDimensions (uint32_t width, int32_t height) {
    matrix ortho = matrix::calculateOrtho(width, height, this->width, this->height);
    triangle_shader.setuniform("projection", 1, true, ortho);
    point_shader.setuniform("projection", 1, true, ortho);
    line_shader.setuniform("projection", 1, true, ortho);
    this->width = width;
    this->height = height;
}
void gore::primitiverenderer::updateView (float x, float y, float zoom) {
    matrix view = matrix::calculate2DView(x, y, zoom);
    triangle_shader.setuniform("view", 1, true, view);
    point_shader.setuniform("view", 1, true, view);
    line_shader.setuniform("view", 1, true, view);
}