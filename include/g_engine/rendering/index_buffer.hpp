#pragma once
#include <vector>
#include "renderer.hpp"

namespace gore {
    template <class T>
    class index_buffer {
        private:
            std::vector<T> vertexs; 
            std::vector<GLuint> indexs;
            // object map
        public:
            index_buffer () {

            }
            // rule of 5
            index_buffer (std::vector<T> vertexs, std::vector<GLuint> indexs) {
                this->vertexs = vertexs;
                this->indexs = indexs;
            }
            // copy
            index_buffer (const index_buffer<T>& c) {
                this->vertexs = c.vertexs;
                this->indexs = c.indexs;
            }
            index_buffer<T>& operator=(const index_buffer<T>& c) {
                this->vertexs = c.vertexs;
                this->indexs = c.indexs;
                return *this;
            }
            // move
            index_buffer (index_buffer<T>&& m) {
                this->vertexs = std::move(m.vertexs);
                this->indexs = std::move(m.indexs);
            }
            index_buffer<T>& operator=(index_buffer<T>&& m) {
                this->vertexs = std::move(m.vertexs);
                this->indexs = std::move(m.indexs);
                return *this;
            }
            // access operators
            T& operator[](size_t index) {
                size_t i = indexs[index];
                return vertexs[i];
            }
            std::vector<T>& getVertexs () {
                return vertexs;
            }
            std::vector<T> getVertexs () const {
                return vertexs;
            }
            std::vector<GLuint>& getIndexs () {
                return indexs;
            }
            std::vector<GLuint> getIndexs () const {
                return indexs;
            }
            void addVertex (T ver) {
                vertexs.push_back(ver);
            }
            void addIndex (GLuint i) {
                indexs.push_back(i);
            }
            size_t vertexSize() const {
                return vertexs.size();
            }
            size_t indexSize () const {
                return indexs.size();
            }
    };
}