#pragma once
#include "../util/shader.hpp"
#include "../util/matrix.hpp"
#include <cassert>
#include <type_traits>
namespace gore {
    template <class Derived, class T>
    class renderer {
        protected:
            std::vector<T> vertexs;
            shader shader;
            GLuint vao;
            GLuint vertex_buffer;
            GLuint allocated;
            uint32_t width, height;
            int32_t texture_units;
            GLuint draw_arrays_mode = GL_TRIANGLES;
            bool created = false;
            virtual void shader_setup() = 0;
        public:
            renderer () {
                
            }
            renderer (std::string vertex_shader, std::string fragment_shader, uint32_t width, uint32_t height) {
                static_assert(!std::is_abstract_v<Derived>, "Derived class must implement shader_setup()");
                vertexs.reserve(1000);
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
                allocated = 0;
                glGenBuffers(1, &vertex_buffer);
                shader.compile(vertex_shader.c_str(), fragment_shader.c_str());
                shader.bind();
                this->width = width;
                this->height = height;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);
            }
            void createRenderer () {
                created = true;
                shader_setup();
            }
            virtual void drawBuffer () {
                assert(created && "call createRenderer before use!");
                if (vertexs.empty()) return;
                shader.bind();
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
                if(vertexs.size() > allocated){
                    allocated = vertexs.size();
                    glBufferData(GL_ARRAY_BUFFER, allocated * sizeof(T), &vertexs[0], GL_DYNAMIC_DRAW);
                }else{
                    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexs.size() * sizeof(T), &vertexs[0]);
                }
                glDrawArraysExt(draw_arrays_mode, 0, (GLsizei)vertexs.size());
                vertexs.clear();
                glBindVertexArray(0);
            }
            virtual void setDimensions (uint32_t width, int32_t height) {
                assert(created && "call createRenderer before use!");
                matrix ortho = matrix::calculateOrtho(width, height, this->width, this->height);
                shader.setuniform("projection", 1, true, ortho);
                this->width = width;
                this->height = height;
            }
            virtual void updateView (float x, float y, float zoom) {
                assert(created && "call createRenderer before use!");
                matrix view = matrix::calculate2DView(x, y, zoom);
                shader.setuniform("view", 1, true, view);
            }
    };
}