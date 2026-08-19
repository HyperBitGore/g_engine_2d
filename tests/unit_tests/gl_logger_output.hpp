#pragma once

#include "../../include/g_engine/gl_defines.hpp"
#include <cstdio>

inline void print_gl_logger(const gore::gl_logger& logger) {
    std::printf("  %s: %zu call(s)\n", logger.function_name.c_str(), logger.calls.size());
    for (size_t call_index = 0; call_index < logger.calls.size(); ++call_index) {
        std::printf("    call %zu:", call_index);
        for (uint8_t byte : logger.calls[call_index]) {
            std::printf(" %02x", byte);
        }
        std::printf("\n");
    }
}

inline void print_gl_logger_output() {
    print_gl_logger(gore::draw_arrays_log);
    print_gl_logger(gore::bind_buffer_log);
    print_gl_logger(gore::buffer_data_log);
    print_gl_logger(gore::buffer_sub_data_log);
    print_gl_logger(gore::bind_vertex_array_log);
    print_gl_logger(gore::enable_vertex_attrib_array_log);
    print_gl_logger(gore::vertex_attrib_log);
    print_gl_logger(gore::vertex_attrib_i_log);
    print_gl_logger(gore::bind_buffer_base_log);
    print_gl_logger(gore::draw_elements_log);
}

inline bool check_gl_logger(const char* name, const gore::gl_logger& logger) {
    const bool passed = !logger.calls.empty() && !logger.buffer_data.empty();
    std::printf("  %s logger data: %s\n", name, passed ? "PASS" : "FAIL");
    return passed;
}

template <typename... Args>
bool check_gl_logger_call(const char* name, const gore::gl_logger& logger,
                          const Args&... expected_arguments) {
    const bool passed = logger.hasCallWithPrefix(expected_arguments...);
    std::printf("  %s argument data: %s\n", name, passed ? "PASS" : "FAIL");
    return passed;
}
