#include "primitive_renderer.hpp"
#include "primitive_renderer_shader.hpp"

//triangles
gore::trianglerenderer::trianglerenderer(uint32_t width, uint32_t height) : gore::renderer<gore::trianglerenderer, gore::vec2> (triangle_vertex, triangle_fragment, width, height) {

}
// copy
//sets color for drawing
void gore::trianglerenderer::setColor(gore::vec4 color) {
     shader.setuniform("set_color", color);
}
void gore::trianglerenderer::addTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
}
void gore::trianglerenderer::drawTriangle(vec2 v1, vec2 v2, vec2 v3){
    vertexs.push_back(v1);
    vertexs.push_back(v2);
    vertexs.push_back(v3);
    drawBuffer();
}
//quads
void gore::trianglerenderer::addQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});  
}
void gore::trianglerenderer::drawQuad(vec2 pos, float w, float h){
    vertexs.push_back({pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + w, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + h}); //first triangle tip vertex

    vertexs.push_back({pos.x + w, pos.y + h});
    vertexs.push_back({pos.x, pos.y + h});
    vertexs.push_back({pos.x + w, pos.y});
    drawBuffer();  
}
// filled circle
void gore::trianglerenderer::addCircleFilled(vec2 p, float r){
    float step = 2.0f * M_PI / 720.0f;
    for (float ang = 0; ang < 2.0f * M_PI; ang += step) {
        vertexs.push_back(p);
        vertexs.push_back({p.x + r * cosf(ang), p.y + r * sinf(ang)});
        vertexs.push_back({p.x + r * cosf(ang + step), p.y + r * sinf(ang + step)});
    }
}

void gore::trianglerenderer::drawCircleFilled(vec2 p, float r){
    addCircleFilled(p, r);
    drawBuffer();
}
//points
gore::pointrenderer::pointrenderer(uint32_t width, uint32_t height) : gore::renderer<gore::pointrenderer, gore::vec2>(point_vertex, point_fragment, width, height) {

}
void gore::pointrenderer::setColor(gore::vec4 color) {
    shader.setuniform("set_color", color);
}
void gore::pointrenderer::addPoint(vec2 p){
    vertexs.push_back(p);
}
void gore::pointrenderer::drawPoint(vec2 p){
    vertexs.push_back(p);
    drawBuffer();
}
//lines
gore::linerenderer::linerenderer(uint32_t width, uint32_t height) : gore::renderer<gore::linerenderer, gore::vec2>(line_vertex, line_fragment, width, height) {

}
void gore::linerenderer::setColor(gore::vec4 color) {
    shader.setuniform("set_color", color);
}
void gore::linerenderer::addLine(vec2 p1, vec2 p2){
    vertexs.push_back(p1);
    vertexs.push_back(p2);
}
void gore::linerenderer::drawLine(vec2 p1, vec2 p2){
    addLine(p1, p2);
    drawBuffer();
}
void gore::linerenderer::drawBuffer(){
    assert(created && "call createRenderer before use!");
    glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    shader.bind();
    glBindVertexArray(vao);
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

void gore::linerenderer::setLineWidth(float l){
    glLineWidth(l);
}

void gore::linerenderer::addLinearBezier(vec2 p1, vec2 p2){
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

void gore::linerenderer::drawLinearBezier(vec2 p1, vec2 p2) {
    addLinearBezier(p1, p2);
    drawBuffer();
}

void gore::linerenderer::addQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv){
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

void gore::linerenderer::drawQuadraticBezier(vec2 p1, vec2 p2, vec2 p3, int subdiv) {
    addQuadraticBezier(p1, p2, p3, subdiv);
    drawBuffer();
}

void gore::linerenderer::addCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv){
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
void gore::linerenderer::drawCubicBezier(vec2 p1, vec2 p2, vec2 p3, vec2 p4, int subdiv) {
    addCubicBezier(p1, p2, p3, p4, subdiv);
    drawBuffer();
}

void gore::linerenderer::addCircleOutline(vec2 p, float r, uint32_t segments) {
    float step = 2.0f * M_PI / segments;
    for (size_t i = 0; i < segments; i++) {
        float a0 = i * (float)step;
        float a1 = (i + 1) * (float)step;
        addLine({p.x + r * std::cosf(a0), p.y + r * sinf(a0)}, {p.x + r * std::cosf(a1), p.y + r * std::sinf(a1)});
    }
}

void gore::linerenderer::drawCircleOutline(vec2 p, float r, uint32_t segments) {
    addCircleOutline(p, r, segments);
    drawBuffer();
}

