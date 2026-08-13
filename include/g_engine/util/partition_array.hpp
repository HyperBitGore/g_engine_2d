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
        public:
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
            partition_array() : data(nullptr), total_size(0) {}
            ~partition_array() {
                delete[] data;
            }
        // rule of 5
            partition_array(const partition_array& other)
                : data(nullptr), partition_list(other.partition_list), partitions(other.partitions), total_size(other.total_size) {
                if (other.data && total_size > 0) {
                    data = new uint8_t[total_size];
                    std::copy(other.data, other.data + total_size, data);
                }
            }
            partition_array& operator=(const partition_array& other) {
                if (this != &other) {
                    uint8_t* new_data = nullptr;
                    if (other.data && other.total_size > 0) {
                        new_data = new uint8_t[other.total_size];
                        std::copy(other.data, other.data + other.total_size, new_data);
                    }
                    delete[] data;
                    data = new_data;
                    partition_list = other.partition_list;
                    partitions = other.partitions;
                    total_size = other.total_size;
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
                extend(total_size + size); // extend() copies the old total_size bytes and updates total_size
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
                    }
                    partition_list.erase(partition_list.begin() + index);
                    partitions.erase(it);
                    // fix up map entries that pointed past the erased partition
                    for (auto& [other_key, other_index] : partitions) {
                        if (other_index > index) {
                            --other_index;
                        }
                    }
                    total_size -= size_to_remove;
                }
            }
            void removePartitionData(size_t key, size_t index, size_t size) {
                auto it = partitions.find(key);
                if (it != partitions.end()) {
                    auto& part = partition_list[it->second];
                    // check if index is valid (index/size are byte offsets within the partition)
                    if (index >= part.index) {
                        return; // invalid index, nothing to remove
                    }
                    if (index + size > part.index) {
                        size = part.index - index; // can't remove more than what's there
                    }
                    std::memmove(data + part.offset + index, data + part.offset + index + size, part.index - index - size);
                    part.index -= size;
                    // zero the now-unused tail so stale data isn't uploaded/reused
                    std::fill(data + part.offset + part.index, data + part.offset + part.index + size, static_cast<uint8_t>(0));
                }
            }
    };
}