#include <iostream>

#include "include/util.hpp"
#include "include/string_pool.hpp"
#include "include/fixed_string_pool.hpp"
#include "include/prefix_indexed_string_pool.hpp"
#include "include/prefix_indexed_string_pool_v2.hpp"
#include "include/prefix_indexed_string_pool_v3.hpp"
#include "include/front_coded_dictionary.hpp"
#include "include/prefix_indexed_front_coded_dictionary.hpp"

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

    static const uint64_t min_string_len = 8 + 1;
    static const uint64_t max_string_len = 256 + 1;
    std::vector<std::string> strings =
        read_string_collection(argv[1], min_string_len, max_string_len);
    // note: strings should be already sorted
    // std::sort(strings.begin(), strings.end());
    uint64_t n = strings.size();
    uint64_t num_queries = std::min<uint64_t>(3000000, n);
    splitmix64 random(13);
    std::vector<uint64_t> queries(num_queries);
    for (uint64_t i = 0; i != num_queries; ++i) queries[i] = random.next() % n;

    // for (auto& s : strings) s.resize(prefix_size);

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
        std::cout << "bytes: " << pool.bytes() << std::endl;
    }

    // {
    //     // measure time for binary search on contiguous fixed-size strings
    //     fixed_string_pool<prefix_size> pool(n);
    //     for (auto const& s : strings) pool.append(s);
    //     uint64_t sum = 0;
    //     auto start = std::chrono::high_resolution_clock::now();
    //     for (auto q : queries) sum += pool.lower_bound(strings[q]);
    //     auto stop = std::chrono::high_resolution_clock::now();
    //     auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
    //     std::cout << "elapsed " << elapsed.count() << std::endl;
    //     std::cout << "##ignore " << sum << std::endl;
    // }

    {
        // measure time for binary search on prefix_indexed_string_pool
        prefix_indexed_string_pool::builder builder(n);
        prefix_indexed_string_pool pool;
        builder.build(strings.begin(), strings.size());
        builder.build(pool);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings[q]);
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;

        sum = 0;
        start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings, strings[q]);
        stop = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
        std::cout << "bytes: " << pool.bytes() << std::endl;
    }

    // {
    //     // measure time for binary search on prefix_indexed_string_pool_v2 (prefixes of 16 bytes,
    //     // instead of 8)
    //     prefix_indexed_string_pool_v2::builder builder(n);
    //     prefix_indexed_string_pool_v2 pool;
    //     builder.build(strings.begin(), strings.size());
    //     builder.build(pool);
    //     uint64_t sum = 0;
    //     auto start = std::chrono::high_resolution_clock::now();
    //     for (auto q : queries) sum += pool.lower_bound(strings[q]);
    //     auto stop = std::chrono::high_resolution_clock::now();
    //     auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
    //     std::cout << "elapsed " << elapsed.count() << std::endl;
    //     std::cout << "##ignore " << sum << std::endl;
    // }

    {
        // measure time for binary search on prefix_indexed_string_pool_v3 that assumes all strings
        // to be longer than 8
        prefix_indexed_string_pool_v3::builder builder(n);
        prefix_indexed_string_pool_v3 pool;
        builder.build(strings.begin(), strings.size());
        builder.build(pool);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += pool.lower_bound(strings[q]);
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
        std::cout << "bytes: " << pool.bytes() << std::endl;
    }

    {
        // measure time for binary search on a front_coded_dictionary
        typedef front_coded_dictionary<16> fc_dict_type;
        fc_dict_type::builder builder;
        fc_dict_type dict;
        builder.build(strings.begin(), strings.size());
        builder.build(dict);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += dict.lower_bound(byte_range_from_string(strings[q]));
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    {
        // measure time for binary search on a prefix_indexed_front_coded_dictionary
        typedef prefix_indexed_front_coded_dictionary<16> fc_dict_type;
        fc_dict_type::builder builder;
        fc_dict_type dict;
        builder.build(strings.begin(), strings.size());
        builder.build(dict);
        uint64_t sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto q : queries) sum += dict.lower_bound(byte_range_from_string(strings[q]));
        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
        std::cout << "elapsed " << elapsed.count() << std::endl;
        std::cout << "##ignore " << sum << std::endl;
    }

    return 0;
}