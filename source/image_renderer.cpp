#include "image_renderer.h"

void ImageRenderer::addImageVertex(vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
}

void ImageRenderer::addImageVertex(vec2 pos, vec2 dim, float rot){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
}
//first two x,y in uv is starting position in image and z, w are width and height for the uvs
void ImageRenderer::addImageVertex(vec2 pos, vec2 dim, vec4 uvs, float rot){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, rot, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, rot, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, rot, pos.x, pos.y}); //top righjt
}

void ImageRenderer::drawBuffer(IMG img){
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawBuffer(GLuint texture){
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.bind();
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}
void ImageRenderer::drawTexture(GLuint texture, vec2 pos, vec2 dim, vec4 uvs){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y}); //top righjt
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.bind();
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
         allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawImage(IMG img, vec2 pos, vec2 dim){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, 0.0f, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f, 0.0f, pos.x, pos.y});
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawImage(IMG img, vec2 pos, vec2 dim, vec4 uvs){
    vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y, 0.0f, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y,0.0f, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //first triangle tip vertex


    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom right
    vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w, 0.0f, pos.x, pos.y}); //bottom left
    vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y, 0.0f, pos.x, pos.y}); //top righjt
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawTextureRotated(GLuint texture, vec2 pos, vec2 dim, float rot){
      vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
        allocated = vertexs.size();
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();
}

void ImageRenderer::drawImageRotated(IMG img, vec2 pos, vec2 dim, float rot){
    vertexs.push_back({pos.x, pos.y, 0.0f, 0.0f, rot, pos.x, pos.y}); //first triangle top left vertex
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y}); //first triangel top right
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f, rot, pos.x, pos.y}); //first triangle tip vertex

    vertexs.push_back({pos.x + dim.x, pos.y + dim.y, 1.0f, 1.0f, rot, pos.x, pos.y});
    vertexs.push_back({pos.x, pos.y + dim.y, 0.0f, 1.0f,rot, pos.x, pos.y});
    vertexs.push_back({pos.x + dim.x, pos.y, 1.0f, 0.0f,rot, pos.x, pos.y});
    glActiveTexture_g(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img->tex);
    shader.bind();
    //shader.setuniform("mtexture", img->tex);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    if(vertexs.size() > allocated){
         glBufferData_g(GL_ARRAY_BUFFER, vertexs.size() * sizeof(ivertex), vertexs.data(), GL_DYNAMIC_DRAW);
    }else{
         glBufferSubData_g(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(ivertex), &vertexs[0]);
    }
    glDrawArrays_g(GL_TRIANGLES, 0, vertexs.size());
    glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
    vertexs.clear();

}

ImageRenderer::ImageRenderer(size_t w, size_t h) {
    const char* vertex_shader = "#version 450 core\n"
        "\n"
        "layout(location = 0) in vec2 pos;\n"
        "layout(location = 1) in vec2 uv;\n"
        "layout(location = 2) in float ang;\n"
        "layout(location = 3) in vec2 rot_p;\n"
        "uniform vec2 screen;\n"
        "out vec2 tex_coord;\n"
        "void main(){\n"
        "    //vec2 pre = vec2(pos.x - rot_p.x, pos.y - rot_p.y); //moving origin to rotation point\n"
        "    //pre = vec2(pre.x*cos(ang)-pre.y*sin(ang), pre.y*cos(ang) + pre.x*sin(ang));\n"
        "    //vec2 p = vec2(pre.x + rot_p.x, pre.y + rot_p.y);\n"
        "    //doing the rotation before conversion causes warpage\n"
        "    vec2 p = pos;\n"
        "    vec2 rot = rot_p;\n"
        "    rot /= screen;\n"
        "    rot = (rot * 2.0) - 1;\n"
        "    p /= screen;\n"
        "    p = (p * 2.0) - 1;\n"
        "    vec2 pre = p - rot;\n"
        "    pre = vec2(pre.x*cos(ang)-pre.y*sin(ang), pre.y*cos(ang) + pre.x*sin(ang));\n"
        "    pre = pre + rot;\n"
        "    p = pre;\n"
        "    tex_coord = uv;\n"
        "    gl_Position = vec4(p.x, -p.y, 0.0, 1.0);\n"
        "}\n"
        "";
    const char* fragment_shader = "#version 450 core\n"
        "out vec4 color;\n"
        "in vec2 tex_coord;\n"
        "uniform sampler2D mtexture;\n"
        "void main(){\n"
        "    color = texture(mtexture, tex_coord);\n"
        "}";
    allocated = 0;
    //shader.compile(std::string("img.vs"), std::string("img.fs"));
    shader.compile(vertex_shader, fragment_shader);
    glGenVertexArrays_g(1, &vao);
    glGenBuffers_g(1, &vertex_buffer);
    glBindVertexArray_g(vao);
    glBindBuffer_g(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray_g(0);
    glVertexAttribPointer_g(0, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)0); //position
    glEnableVertexAttribArray_g(1);
    glVertexAttribPointer_g(1, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 2)); //uv
    glEnableVertexAttribArray_g(2);
    glVertexAttribPointer_g(2, 1, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 4)); //rotation
    glEnableVertexAttribArray_g(3);
    glVertexAttribPointer_g(3, 2, GL_FLOAT, GL_FALSE, sizeof(ivertex), (void*)(sizeof(float) * 5)); //rotation point
    shader.bind();
    shader.setuniform("screen", {(float)w, (float)h});
    shader.setuniform("mtexture", (GLuint)0);
}