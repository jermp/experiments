#include <iostream>
#include <vector>
#include <random>
#include <chrono>

constexpr uint64_t LINE_SIZE = 64;

struct cache {
    cache(uint64_t s, uint64_t w)
        : size(s), ways(w), sets((size / LINE_SIZE) / ways), counters(sets, 0) {
        if (size % LINE_SIZE != 0) {
            throw std::runtime_error("cache size must be divisible by the line size");
        }
    }

    template <typename T>
    void map(T const& x) {
        uint64_t address = reinterpret_cast<uint64_t>(&x);
        uint64_t set = (address / LINE_SIZE) % sets;  // or just take log2(sets) lowest bits, after
                                                      // shifting to the right by log2(LINE_SIZE)
        counters[set] += 1;
    }

    void print_usage() const {
        std::cout << "cache specification:\n";
        std::cout << "  size " << size << "\n";
        std::cout << "  ways " << ways << "\n";
        std::cout << "  sets " << sets << "\n";
        std::cout << "cache usage:\n";
        for (uint64_t i = 0; i != sets; ++i) {
            // std::cout << "set " << i << ": " << counters[i] << "\n";
            std::cout << counters[i] << " ";
        }
        std::cout << std::endl;
    }

    void clear() {
        std::fill(counters.begin(), counters.end(), 0);
    }

    uint64_t accesses() const {
        return std::accumulate(counters.begin(), counters.end(), 0);
    }

private:
    uint64_t size;  // in bytes
    uint64_t ways;
    uint64_t sets;
    std::vector<uint64_t> counters;  // the number of addresses that are mapped to a set
};

#include "include/segment_trees.hpp"
#include "include/fenwick_trees.hpp"

#define TEST                                                                               \
    tree.build(input.data(), input.size());                                                \
    int64_t sum = 0;                                                                       \
    auto start = clock_t::now();                                                           \
    for (auto q : queries) { sum += tree.sum(q, L1); }                                     \
    auto stop = clock_t::now();                                                            \
    auto elapsed = std::chrono::duration_cast<duration_t>(stop - start);                   \
    std::cout << "# ignore " << sum << std::endl;                                          \
    std::cout << "elapsed time: " << elapsed.count() / 1000 << " [millisec]" << std::endl; \
    std::cout << "cache usage:\n";                                                         \
    L1.print_usage();                                                                      \
    std::cout << "accesses " << L1.accesses() << std::endl;                                \
    std::cout << "iterations " << tree.iterations << std::endl;                            \
    std::cout << "size " << tree.size() << std::endl;

int main(int argc, char const** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <size>" << std::endl;
        return 1;
    }

    constexpr unsigned seed = 192390;
    std::mt19937_64 rng(seed);

    cache L1(32 * 1024, 8);  // typical parameters for L1 cache

    uint64_t n = std::stoull(argv[1]);
    if (!n) return 1;
    std::vector<int64_t> input(n, 1);

    constexpr uint64_t num_queries = 10000;
    std::uniform_int_distribution<uint64_t> distr(0, n - 1);
    std::vector<uint64_t> queries(num_queries);
    std::generate(queries.begin(), queries.end(), [&] { return distr(rng); });

    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::microseconds duration_t;

    {
        std::cout << "=== fenwick_tree\n";
        fenwick_tree tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== fenwick_tree_holes\n";
        fenwick_tree_holes tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== segment_tree_topdown\n";
        segment_tree_topdown tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== segment_tree_bottomup\n";
        segment_tree_bottomup tree;
        TEST
    }

    return 0;
}