#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
uniform vec2 screen;
out vec2 tex_coord;
void main(){
    vec2 p = pos;
    p /= screen;
    p = (p * 2.0) - 1;
    tex_coord = uv;
    gl_Position = vec4(p.x, -p.y, 0.0, 1.0);
}
