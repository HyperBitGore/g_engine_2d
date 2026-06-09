#version 450 core
out vec4 color;
in vec2 tex_coord;
uniform sampler2D texture;
void main(){
    color = vec4(1.0, 0.1, 0.3, 0.0);
}