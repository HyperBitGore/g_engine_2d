#version 450 core
out vec4 color;
in vec2 tex_coord;
uniform sampler2D mtexture;
void main(){
    //color = vec4(1.0, 0.0, 0.0, 0.0);
    color = vec4(vec3(1.0 - texture(mtexture, tex_coord)), 1.0);
}