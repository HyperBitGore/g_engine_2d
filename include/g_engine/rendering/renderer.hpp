#pragma once
#include "../util/shader.hpp"
#include "../util/matrix.hpp"
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>
namespace gore {
    class renderer_base {
        public:
        virtual ~renderer_base () = default;
        virtual void drawBuffer () = 0;
        virtual void updateDimensions (uint32_t w, uint32_t h) = 0;
        virtual void updateView (gore::vec2 pos, float zoom) = 0;
        virtual void updateView (gore::vec3 camera_pos, gore::vec3 camera_target, gore::vec3 upVector) = 0;
    };

    template <class Derived, class T>
    class renderer : public renderer_base {
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
            void createRenderer () {
                created = true;
                shader_setup();
            }
            renderer () {
                
            }
            renderer (std::string vertex_shader, std::string fragment_shader, uint32_t width, uint32_t height, bool file = false) {
                static_assert(!std::is_abstract_v<Derived>, "Derived class must implement shader_setup()");
                vertexs.reserve(1000);
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
                allocated = 0;
                glGenBuffers(1, &vertex_buffer);
                if (file) {
                    shader.compile(vertex_shader, fragment_shader);
                } else {
                    shader.compile(vertex_shader.c_str(), fragment_shader.c_str());
                }
                shader.bind();
                this->width = width;
                this->height = height;
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);
            }
        public:
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
            virtual void updateDimensions (uint32_t width, uint32_t height) {
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
            virtual void updateView (gore::vec2 pos, float zoom) {
                assert(created && "call createRenderer before use!");
                matrix view = matrix::calculate2DView(pos.x, pos.y, zoom);
                shader.setuniform("view", 1, true, view);
            }
            virtual void updateView (gore::vec3 camera_pos, gore::vec3 camera_target, gore::vec3 upVector) {
                assert(created && "call createRenderer before use!");
                gore::matrix view = gore::matrix::lookat(camera_pos, camera_target, upVector);
                shader.setuniform("view", 1, true, view);
            }
            template<class FactoryDerived = Derived, class... Args>
            static std::unique_ptr<FactoryDerived> create(Args&&... args) {
                static_assert(std::is_base_of_v<Derived, FactoryDerived>,
                    "Factory type must derive from the renderer's root derived type");
                std::unique_ptr<FactoryDerived> r = std::unique_ptr<FactoryDerived>(
                    new FactoryDerived(std::forward<Args>(args)...)
                );
                r->createRenderer();
                return r;
            }
    };
}