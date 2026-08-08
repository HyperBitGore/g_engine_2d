#pragma once
#include "renderer.hpp"

namespace gore{

    class texture_unit_manager {
        private:
            std::unordered_map<GLuint, GLuint> texture_unit_map;
            std::vector<GLint> samplers;
            uint32_t current_unit;
            std::vector<GLuint> bound_textures;
            size_t draw_split = 1;
            GLint texture_units;
        public:
            // rule of five
            texture_unit_manager () {
                current_unit = 0;
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
            }
            texture_unit_manager (const texture_unit_manager& tm) {
                this->texture_unit_map = tm.texture_unit_map;
                this->samplers = tm.samplers;
                this->current_unit = tm.current_unit;
                this->bound_textures = tm.bound_textures;
            }
            texture_unit_manager (texture_unit_manager&& tm) {
                this->texture_unit_map = std::move(tm.texture_unit_map);
                this->samplers = std::move(tm.samplers);
                this->current_unit = std::move(tm.current_unit);
                this->bound_textures = std::move(tm.bound_textures);
            }
            texture_unit_manager& operator=(const texture_unit_manager& tm) {
                this->texture_unit_map = tm.texture_unit_map;
                this->samplers = tm.samplers;
                this->current_unit = tm.current_unit;
                this->bound_textures = tm.bound_textures;
                return *this;
            }
            texture_unit_manager& operator=(texture_unit_manager&& tm) {
                this->texture_unit_map = std::move(tm.texture_unit_map);
                this->samplers = std::move(tm.samplers);
                this->current_unit = std::move(tm.current_unit);
                this->bound_textures = std::move(tm.bound_textures);
                return *this;
            }
            GLuint getTextureUnit (GLuint texture) {
                auto it = texture_unit_map.find(texture);
                if (it == texture_unit_map.end()) {
                    uint32_t f_unit = GL_TEXTURE0 + current_unit;
                    texture_unit_map.emplace(texture, current_unit);
                    glActiveTexture(f_unit);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    samplers.push_back(current_unit);
                    bound_textures.push_back(texture);
                    current_unit += 1;
                    if (current_unit >= texture_units) {
                        draw_split++;
                    }
                    return current_unit - 1;
                    
                }
                return it->second;
            }
            bool textureBinded (GLuint texture) {
                auto it = texture_unit_map.find(texture);
                return it != texture_unit_map.end();
            }
            void setTextureSamplers (const std::string& sampler_array_target, gore::shader& shader) {
                // rebind textures in case another renderer clobbered the units between frames
                for (size_t i = 0; i < samplers.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + samplers[i]);
                    glBindTexture(GL_TEXTURE_2D, bound_textures[i]);
                }
                shader.setuniform(sampler_array_target, samplers.size(), samplers.data());
            }
            std::vector<GLint>& getSamplers () {
                return samplers;
            }
            void clearUnits () {
                samplers.clear();
                bound_textures.clear();
            }
            size_t drawsplits () {
                return draw_split;
            }
            size_t currentUnit () {
                return current_unit;
            }
    };

}