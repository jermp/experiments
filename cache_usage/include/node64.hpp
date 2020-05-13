#pragma once

#include <cassert>
#include "common.hpp"

struct node64 {
    static constexpr uint64_t fanout = 64;
    static constexpr uint64_t segment_size = 8;
    static constexpr uint64_t bytes = (fanout + segment_size) * sizeof(int64_t);

    node64() {}  // do not initialize

    template <typename T>
    static void build(T const* input, uint8_t* out) {
        build_node_prefix_sums(input, out, segment_size, bytes);
    }

    node64(uint8_t* ptr) {
        at(ptr);
    }

    inline void at(uint8_t* ptr) {
        summary = reinterpret_cast<int64_t*>(ptr);
        keys = reinterpret_cast<int64_t*>(ptr + segment_size * sizeof(int64_t));
    }

    int64_t sum(uint64_t i, cache& c) {
        assert(i < fanout);
        c.map(summary[i / segment_size]);
        c.map(keys[i]);
        return summary[i / segment_size] + keys[i];
    }

private:
    int64_t* summary;
    int64_t* keys;
};
