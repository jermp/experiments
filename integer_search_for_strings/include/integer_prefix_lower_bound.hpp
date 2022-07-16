#pragma once

#include <iterator>
#include <vector>
#include <string>
#include <immintrin.h>  // for __builtin_bswap64

inline uint64_t string_prefix_to_uint64(std::string const& s) {
    return __builtin_bswap64(*reinterpret_cast<uint64_t const*>(s.data()));
}

inline bool compare_integer_prefix(std::string const& str, uint64_t prefix) {
    uint64_t x = string_prefix_to_uint64(str);
    return x < prefix;
}

template <typename Iterator>
Iterator integer_prefix_lower_bound(Iterator first, Iterator last, std::string const& val) {
    uint64_t target_prefix = string_prefix_to_uint64(val);
    Iterator it;
    typename std::iterator_traits<Iterator>::difference_type count, step;
    count = std::distance(first, last);

    /*

        the problem with this approach is that
        it does not point to the *first* distinct prefix,
        so then we cannot complete...

    */
    while (count > 0) {
        it = first;
        step = count / 2;
        std::advance(it, step);
        bool less = compare_integer_prefix(*it, target_prefix);
        if (less) {
            first = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }
    return first;

    // it = first;
    // uint64_t n = count;
    // for (uint64_t i = 0; i != n; ++i, ++it) {
    //     if (*it >= val) return it;
    // }
    // return it;

    // while (count > 0) {
    //     it = first;
    //     step = count / 2;
    //     std::advance(it, step);
    //     if (*it < val) {
    //         first = ++it;
    //         count -= step + 1;
    //     } else {
    //         count = step;
    //     }
    // }
    // return first;
}
