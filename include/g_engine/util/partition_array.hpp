#pragma once
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace gore {
    class partition_array {
        private:
            uint8_t* data;
            struct partition {
                size_t offset;
                size_t size;
                size_t index;
            };
            std::vector<partition> partition_list;
            std::unordered_map<size_t, size_t> partitions; // maps key to index in partition_list
            size_t total_size;
            void extend(size_t new_size) {
                const size_t old_size = total_size;
                uint8_t* new_data = new uint8_t[new_size];
                std::fill(new_data, new_data + new_size, 0);
                if (data && old_size > 0) {
                    std::copy(data, data + old_size, new_data);
                }
                delete[] data;
                data = new_data;
                total_size = new_size;
            }
            void extendPartition(size_t key, size_t byte_additional_size) {
                auto it = partitions.find(key);
                if (it != partitions.end()) {
                    uint8_t* new_data = new uint8_t[total_size + byte_additional_size];
                    std::fill(new_data, new_data + total_size + byte_additional_size, 0);
                    auto& part = partition_list[it->second];
                    std::memcpy(new_data, data, part.offset + part.size);
                    for (size_t i = it->second + 1; i < partition_list.size(); ++i) {
                        auto& next_part = partition_list[i];
                        std::memcpy(new_data + next_part.offset + byte_additional_size, data + next_part.offset, next_part.size);
                        next_part.offset += byte_additional_size;
                    }
                    delete[] data;
                    data = new_data;
                    part.size += byte_additional_size;
                    total_size += byte_additional_size;
                }
            }
        public:
            partition_array() : data(nullptr), total_size(0) {}
            ~partition_array() {
                delete[] data;
            }
        // rule of 5
            partition_array(const partition_array& other) : data(nullptr), total_size(0) {
                for (const auto& [key, index] : other.partitions) {
                    const auto& part = other.partition_list[index];
                    addPartition(key, part.size);
                }
            }
            partition_array& operator=(const partition_array& other) {
                if (this != &other) {
                    delete[] data;
                    data = nullptr;
                    total_size = 0;
                    partitions.clear();
                    for (const auto& [key, index] : other.partitions) {
                        const auto& part = other.partition_list[index];
                        addPartition(key, part.size);
                    }
                }
                return *this;
            }
            partition_array(partition_array&& other) noexcept
                : data(other.data), partition_list(std::move(other.partition_list)), partitions(std::move(other.partitions)), total_size(other.total_size) {
                other.data = nullptr;
                other.total_size = 0;
            }
            partition_array& operator=(partition_array&& other) noexcept {
                if (this != &other) {
                    delete[] data;
                    data = other.data;
                    partition_list = std::move(other.partition_list);
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
                partition_list.push_back({total_size, size, 0});
                partitions[key] = partition_list.size() - 1;
                total_size += size;
                extend(total_size);
                return true;
            }
            uint8_t* getData () {
                return data;
            }
            size_t size () const {
                return total_size;
            }
            void addPartitionData(size_t key, const uint8_t* new_data, size_t size) {
                auto it = partitions.find(key);
                if (it != partitions.end()) {
                    auto& part = partition_list[it->second];
                    if (part.index + size > part.size) {
                        extendPartition(key, part.index + size - part.size);
                    }
                    std::copy(new_data, new_data + size, data + part.offset + part.index);
                    part.index += size;
                }
            }
            void removePartition(size_t key) {
                auto it = partitions.find(key);
                if (it != partitions.end()) {
                    size_t index = it->second;
                    auto& part = partition_list[index];
                    size_t size_to_remove = part.size;
                    for (size_t i = index + 1; i < partition_list.size(); ++i) {
                        auto& next_part = partition_list[i];
                        std::memmove(data + next_part.offset - size_to_remove, data + next_part.offset, next_part.size);
                        next_part.offset -= size_to_remove;
                        next_part.index--;
                    }
                    partition_list.erase(partition_list.begin() + index);
                    partitions.erase(it);
                    total_size -= size_to_remove;
                }
            }
    };
}