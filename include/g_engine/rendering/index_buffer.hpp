#pragma once
#include <unordered_map>
#include <vector>
#include "renderer.hpp"

namespace gore {
    template <class T>
    class index_buffer {
        private:
            std::vector<T> vertexs; 
            std::vector<GLuint> indexs;
            // object map
            std::unordered_map<T, size_t> vertex_map;
            size_t dupes = 0;
        public:
            index_buffer () {

            }
            // rule of 5
            index_buffer (std::vector<T> vertexs, std::vector<GLuint> indexs) {
                this->vertexs = std::move(vertexs);
                this->indexs = std::move(indexs);
                vertex_map.reserve(this->vertexs.size());
                for (size_t i = 0; i < this->vertexs.size(); i++) {
                    vertex_map.emplace(this->vertexs[i], i); // no-op if key already present
                }
            }
            // copy
            index_buffer (const index_buffer<T>& c) {
                this->vertexs = c.vertexs;
                this->indexs = c.indexs;
                this->vertex_map = c.vertex_map;
            }
            index_buffer<T>& operator=(const index_buffer<T>& c) {
                this->vertexs = c.vertexs;
                this->indexs = c.indexs;
                this->vertex_map = c.vertex_map;
                return *this;
            }
            // move
            index_buffer (index_buffer<T>&& m) {
                this->vertexs = std::move(m.vertexs);
                this->indexs = std::move(m.indexs);
                this->vertex_map = std::move(m.vertex_map);
            }
            index_buffer<T>& operator=(index_buffer<T>&& m) {
                this->vertexs = std::move(m.vertexs);
                this->indexs = std::move(m.indexs);
                this->vertex_map = std::move(m.vertex_map);
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
            size_t dupeCount () {
                return dupes;
            }
            bool addVertex (T vertex) {
                auto pos = vertex_map.find(vertex);
                if (pos == vertex_map.end()) {
                    vertexs.push_back(vertex);
                    indexs.push_back(vertexs.size() - 1);
                    vertex_map.emplace(vertex, vertexs.size() - 1);
                    return true;
                }
                dupes++;
                indexs.push_back(pos->second);
                return false;
            }
            size_t vertexSize() const {
                return vertexs.size();
            }
            size_t indexSize () const {
                return indexs.size();
            }
    };
}