#pragma once
#include "renderer.hpp"
#include <algorithm>

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
            bool full () const {
                return current_unit >= static_cast<uint32_t>(texture_units);
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
                texture_unit_map.clear();
                samplers.clear();
                bound_textures.clear();
                current_unit = 0;
                draw_split = 1;
            }
            size_t drawsplits () {
                return draw_split;
            }
            size_t currentUnit () {
                return current_unit;
            }
    };
    // simplify this to just a get and a sampler array access
    class bindless_texture_manager {
        private:
            std::unordered_map<GLuint, size_t> texture_handle_map;
            std::vector<GLuint64> samplers;
        public:
            bindless_texture_manager() = default;
            bindless_texture_manager (const bindless_texture_manager& btm) {
                this->texture_handle_map = btm.texture_handle_map;
                this->samplers = btm.samplers;
            }
            bindless_texture_manager (bindless_texture_manager&& btm) {
                this->texture_handle_map = std::move(btm.texture_handle_map);
                this->samplers = std::move(btm.samplers);
            }
            bindless_texture_manager& operator=(const bindless_texture_manager& btm) {
                this->texture_handle_map = btm.texture_handle_map;
                this->samplers = btm.samplers;
                return *this;
            }
            bindless_texture_manager& operator=(bindless_texture_manager&& btm) {
                this->texture_handle_map = std::move(btm.texture_handle_map);
                this->samplers = std::move(btm.samplers);
                return *this;
            }
            ~bindless_texture_manager() {
                for (auto& pair : texture_handle_map) {
                    auto& handle = samplers[pair.second];
                    if (glIsTextureHandleResidentARB(handle)) {
                        glMakeTextureHandleNonResidentARB(handle);
                    }
                }
            }
            size_t getTextureIndex (GLuint texture) {
                auto it = texture_handle_map.find(texture);
                if (it == texture_handle_map.end()) {
                    GLuint64 handle = glGetTextureHandleARB(texture);
                    glMakeTextureHandleResidentARB(handle);
                    size_t index = samplers.size();
                    texture_handle_map.emplace(texture, index);
                    samplers.push_back(handle);
                    return index;
                }
                return it->second;
            }
            void removeTextureHandle (GLuint texture) {
                auto it = texture_handle_map.find(texture);
                if (it != texture_handle_map.end()) {
                    GLuint64 handle = samplers[it->second];
                    if (glIsTextureHandleResidentARB(handle)) {
                        glMakeTextureHandleNonResidentARB(handle);
                    }
                    texture_handle_map.erase(it);
                    samplers.erase(std::remove(samplers.begin(), samplers.end(), handle), samplers.end());
                }
            }
            GLuint64 getTextureHandle (GLuint texture) {
                auto it = texture_handle_map.find(texture);
                if (it != texture_handle_map.end()) {
                    return samplers[it->second];
                }
                return 0;
            }
            void makeTextureHandleNonResident (GLuint64 handle) {
                glMakeTextureHandleNonResidentARB(handle);
            }
            void makeTextureHandleResident (GLuint64 handle) {
                glMakeTextureHandleResidentARB(handle);
            }
            std::vector<GLuint64>& getSamplers () {
                return samplers;
            }
            void clear() {
                for (auto& i : samplers) {
                    if (glIsTextureHandleResidentARB(i)) {
                        glMakeTextureHandleNonResidentARB(i);
                    }
                }
                samplers.clear();
                texture_handle_map.clear();
            }
    };

}