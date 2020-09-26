#include <iostream>
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

// g++ -std=c++17 -O3 -march=native integer_search_for_strings.cpp -o
// integer_search_for_strings

#include "include/fixed_string_pool.hpp"

static const uint64_t prefix_size = 8;
typedef std::chrono::microseconds duration_type;

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
uint64_t string_to_uint64(std::string const& s) {
    std::string tmp(s.substr(0, prefix_size));
    if (s.size() < prefix_size) {
        for (uint64_t i = 0; i != prefix_size - s.size(); ++i) tmp.push_back(0);
    }
    std::reverse(tmp.begin(), tmp.end());
    return *reinterpret_cast<uint64_t const*>(tmp.data());
}

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

int main(int argc, char const** argv) {
    if constexpr (prefix_size > 8) {
        std::cout << "prefix_size must be 8 at most" << std::endl;
        return 1;
    }

    if (argc < 2) {
        std::cout << argv[0] << " strings_filename" << std::endl;
        return 1;
    }

    std::vector<std::string> strings = read_string_collection(argv[1]);
    // note: strings should be already sorted
    // std::sort(strings.begin(), strings.end());
    uint64_t n = strings.size();
    uint64_t num_queries = std::min<uint64_t>(1000000, n);
    splitmix64 random(13);
    std::vector<uint64_t> queries(num_queries);
    for (uint64_t i = 0; i != num_queries; ++i) queries[i] = random.next() % n;

    for (auto& s : strings) s.resize(prefix_size);
    {
        // measure time for binary search on std::vector<std::string>
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) {
            auto it = std::lower_bound(strings.begin(), strings.end(), strings[q]);
            sum += std::distance(strings.begin(), it);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    fixed_string_pool<prefix_size> pool(n);
    for (auto const& s : strings) pool.append(s);
    {
        // measure time for binary search on contiguous fixed-size strings
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings[q]);
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    std::vector<uint64_t> uint64_vec;
    uint64_vec.reserve(strings.size());
    for (auto const& s : strings) {
        uint64_t x = string8_to_uint64(s);
        uint64_vec.push_back(x);
        // std::cout << x << std::endl;
        // std::cout << std::bitset<64>(x) << std::endl;
    }

    bool is_sorted = std::is_sorted(uint64_vec.begin(), uint64_vec.end());
    if (is_sorted) std::cout << "integer vector IS SORTED" << std::endl;

    {
        // measure time for binary search on uint64_t
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) {
            uint64_t x = string8_to_uint64(strings[q]);
            auto it = std::lower_bound(uint64_vec.begin(), uint64_vec.end(), x);
            sum += std::distance(uint64_vec.begin(), it);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    return 0;
}