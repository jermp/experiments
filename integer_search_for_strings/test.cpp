#include <iostream>

// g++ -std=c++17 -O3 -march=native integer_search_for_strings.cpp -o
// integer_search_for_strings

#include "include/util.hpp"
#include "include/string_pool.hpp"
#include "include/fixed_string_pool.hpp"

static const uint64_t prefix_size = 8;
typedef std::chrono::microseconds duration_type;

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
    uint64_t num_queries = std::min<uint64_t>(3000000, n);
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

    {
        // measure time for binary search on contiguous strings
        string_pool::builder builder(n);
        string_pool pool;
        builder.build(strings.begin(), strings.size());
        builder.build(pool);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings[q]);
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    {
        // measure time for binary search on contiguous fixed-size strings
        fixed_string_pool<prefix_size> pool(n);
        for (auto const& s : strings) pool.append(s);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings[q]);
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    {
        // measure time for binary search on uint64_t
        std::vector<uint64_t> uint64_vec;
        uint64_vec.reserve(strings.size());

        // all prefixes, possibly repeated
        for (auto const& s : strings) {
            uint64_t x = string8_to_uint64(s);
            uint64_vec.push_back(x);
            // std::cout << x << std::endl;
            // std::cout << std::bitset<64>(x) << std::endl;
        }

        // keep only distinct prefixes
        auto it = std::unique(uint64_vec.begin(), uint64_vec.end());
        uint64_vec.resize(std::distance(uint64_vec.begin(), it));
        std::cout << "num. distinct prefixes: " << uint64_vec.size() << " ("
                  << (uint64_vec.size() * 100.0) / strings.size() << "%)" << std::endl;
        bool is_sorted = std::is_sorted(uint64_vec.begin(), uint64_vec.end());
        if (is_sorted) std::cout << "integer vector IS SORTED" << std::endl;

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