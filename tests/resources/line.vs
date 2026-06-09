#version 450 core

layout(location = 0) in vec2 pos;
uniform vec2 screen;

void main(){
    vec2 p = pos;
    p /= screen;
    p = (p * 2.0) - 1;
    gl_Position = vec4(p.x, p.y, 0.0, 1.0);
}
