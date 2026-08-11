#pragma once
#include <cstdint>
#include <cstddef>
#include <unordered_map>

namespace gore {
    class partition_array {
        private:
            uint8_t* data;
            struct partition {
                size_t offset;
                size_t size;
            };
            std::unordered_map<size_t, partition> partitions;
            size_t total_size;
            void extend(size_t new_size) {
                uint8_t* new_data = new uint8_t[new_size];
                if (data) {
                    std::copy(data, data + total_size, new_data);
                    delete[] data;
                }
                data = new_data;
                total_size = new_size;
            }
        public:
            partition_array() : data(nullptr), total_size(0) {}
            ~partition_array() {
                delete[] data;
            }
        // rule of 5
            partition_array(const partition_array& other) : data(nullptr), total_size(0) {
                for (const auto& [key, part] : other.partitions) {
                    addPartition(key, part.size);
                }
            }
            partition_array& operator=(const partition_array& other) {
                if (this != &other) {
                    delete[] data;
                    data = nullptr;
                    total_size = 0;
                    partitions.clear();
                    for (const auto& [key, part] : other.partitions) {
                        addPartition(key, part.size);
                    }
                }
                return *this;
            }
            partition_array(partition_array&& other) noexcept
                : data(other.data), partitions(std::move(other.partitions)), total_size(other.total_size) {
                other.data = nullptr;
                other.total_size = 0;
            }
            partition_array& operator=(partition_array&& other) noexcept {
                if (this != &other) {
                    delete[] data;
                    data = other.data;
                    partitions = std::move(other.partitions);
                    total_size = other.total_size;
                    other.data = nullptr;
                    other.total_size = 0;
                }
                return *this;
            }
            bool addPartition(size_t key, size_t size) {
                if (partitions.find(key) != partitions.end()) {
                    return false; // partition already exists
                }
                partitions[key] = {total_size, size};
                total_size += size;
                extend(total_size);
                return true;
            }
            uint8_t* getData () {
                return data;
            }
            
    };
}