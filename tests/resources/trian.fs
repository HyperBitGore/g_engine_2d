#version 450 core
out vec4 color;
uniform vec4 set_color;
void main(){
    color = set_color;
}