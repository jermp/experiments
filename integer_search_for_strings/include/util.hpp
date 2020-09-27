#pragma once

#include <cmath>
#include <bitset>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <immintrin.h>  // for __builtin_bswap64
#include <cstring>

struct byte_range {
    uint8_t const* begin;
    uint8_t const* end;
};

inline int byte_range_compare(byte_range l, byte_range r) {
    int size_l = l.end - l.begin;
    int size_r = r.end - r.begin;
    int n = size_l < size_r ? size_l : size_r;
    int cmp =
        strncmp(reinterpret_cast<const char*>(l.begin), reinterpret_cast<const char*>(r.begin), n);
    if (cmp != 0) return cmp;
    return size_l - size_r;
}

byte_range byte_range_from_string(std::string const& str) {
    const uint8_t* buf = reinterpret_cast<uint8_t const*>(str.c_str());
    const uint8_t* end = buf + str.size();  // exclude the null terminator
    return {buf, end};
}

template <uint64_t string_size>
byte_range byte_range_from_string(std::string const& str) {
    const uint8_t* buf = reinterpret_cast<uint8_t const*>(str.c_str());
    const uint8_t* end;
    if (str.size() >= string_size) {
        end = buf + string_size;
    } else {
        end = buf + str.size();  // exclude the null terminator
    }
    return {buf, end};
}

struct splitmix64 {
    splitmix64(uint64_t seed) : x(seed){};

    uint64_t next() {
        uint64_t z = (x += uint64_t(0x9E3779B97F4A7C15));
        z = (z ^ (z >> 30)) * uint64_t(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)) * uint64_t(0x94D049BB133111EB);
        return z ^ (z >> 31);
    }

private:
    uint64_t x;
};

std::vector<std::string> read_string_collection(char const* filename) {
    std::ifstream input(filename);
    std::string s;
    uint64_t max_string_length = 0;
    uint64_t sum_of_lengths = 0;
    std::vector<std::string> strings;
    while (std::getline(input, s)) {
        if (s.size() > max_string_length) max_string_length = s.size();
        sum_of_lengths += s.size();
        strings.push_back(s);
        if (strings.size() % 1000000 == 0) {
            std::cout << "read " << strings.size() << " strings" << std::endl;
        }
    }
    input.close();
    std::cout << "num_strings " << strings.size() << std::endl;
    std::cout << "max_string_length " << max_string_length << std::endl;
    std::cout << "total_length " << sum_of_lengths << std::endl;
    std::cout << "avg_string_length " << std::fixed << std::setprecision(2)
              << static_cast<double>(sum_of_lengths) / strings.size() << std::endl;
    return strings;
}

// generic function
// static const uint64_t prefix_size = 8;
// uint64_t string_to_uint64(std::string const& s) {
//     std::string tmp(s.substr(0, prefix_size));
//     if (s.size() < prefix_size) {
//         for (uint64_t i = 0; i != prefix_size - s.size(); ++i) tmp.push_back(0);
//     }
//     std::reverse(tmp.begin(), tmp.end());
//     return *reinterpret_cast<uint64_t const*>(tmp.data());
// }

// version that assumes a prefix of size 8
uint64_t string8_to_uint64(std::string const& s) {
    // option 1. builtin reverse of bytes
    return __builtin_bswap64(*reinterpret_cast<uint64_t const*>(s.data()));

    // option 2. manual reverse of bytes
    // assert(s.size() >= 8);
    // uint64_t x = (uint64_t(s[0]) << 56) + (uint64_t(s[1]) << 48) + (uint64_t(s[2]) << 40) +
    //              (uint64_t(s[3]) << 32) + (uint64_t(s[4]) << 24) + (uint64_t(s[5]) << 16) +
    //              (uint64_t(s[6]) << 8) + uint64_t(s[7]);
    // return x;
}