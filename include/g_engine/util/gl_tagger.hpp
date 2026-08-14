#pragma once
#include "../gl_defines.hpp"
#include <stdexcept>
#include <string>

// use this object to tag classes for their gl function usage, use it for backup rendering/errors when hardware doesn't support gl functions
namespace gore {
    class render_function_not_supported : public std::runtime_error {
        public:
            explicit render_function_not_supported (const std::string& message) : std::runtime_error (message) {}
    };

    class gl_function_tagger {
        private:
            std::vector<std::string> function_names;
        public:
            // rule of 5
            gl_function_tagger() = default;
            gl_function_tagger(const gl_function_tagger& other) {
                this->function_names = other.function_names;
            }
            gl_function_tagger(gl_function_tagger&& other) noexcept {
                this->function_names = std::move(other.function_names);
            }
            gl_function_tagger& operator=(const gl_function_tagger& other) {
                if (this != &other) {
                    this->function_names = other.function_names;
                }
                return *this;
            }
            gl_function_tagger& operator=(gl_function_tagger&& other) noexcept {
                if (this != &other) {
                    this->function_names = std::move(other.function_names);
                }
                return *this;
            }
            ~gl_function_tagger() = default;
            gl_function_tagger (std::vector<std::string> function_names) {
                this->function_names = function_names;
            }
            
            void tagFunction(const std::string& function_name, bool available) {
                function_names.push_back(function_name);
            }
            void hardwareSupports() {
                for (auto& name : function_names) {
                    if (GetGLFuncAddress(name.c_str()) == nullptr) {
                        throw render_function_not_supported("Function: " + name + " , not supported on this systems drivers!");
                    }
                }
            }
    };
}