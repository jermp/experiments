#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

#include "util.hpp"

/* A pool of strings indexed by their integer prefixes of size 8.
    It is assumed that all strings are of size > 8, so that we store
    common prefixes of size 8 once and keep only the string suffixes.
*/

struct prefix_indexed_string_pool_v3 {
    typedef uint32_t pointer_type;

    struct builder {
        builder(uint64_t num_strings = 0) {
            m_strings_offsets.reserve(num_strings + 1);
            m_strings_offsets.push_back(0);
        }

        template <typename Iterator>
        void build(Iterator begin, uint64_t n) {
            for (uint64_t i = 0; i != n; ++i, ++begin) {
                auto str = *begin;
                assert(str.size() > 8);
                append(byte_range_from_string(str));
                if (m_strings.size() > (uint64_t(1) << (sizeof(pointer_type) * 8))) {
                    throw std::runtime_error(std::to_string(sizeof(pointer_type) * 8) +
                                             " bits per pointers are not enough");
                }

                // keep only distinct integer prefixes
                uint64_t x = string_to_uint<64>(str);
                if (m_prefixes.empty()) {
                    m_pointers.push_back(0);
                    m_prefixes.push_back(x);
                    continue;
                }

                static const uint64_t C = 0;
                if (m_prefixes.back() != x and i - m_pointers.back() > C) {
                    m_pointers.push_back(i);
                    m_prefixes.push_back(x);
                }
            }
            m_pointers.push_back(n);

            std::cout << "num. prefixes: " << m_prefixes.size() << " ("
                      << (m_prefixes.size() * 100.0) / n << "%)" << std::endl;
            assert(std::unique(m_prefixes.begin(), m_prefixes.end()) == m_prefixes.end());
            assert(std::is_sorted(m_prefixes.begin(), m_prefixes.end()));
        }

        void append(byte_range br) {
            m_strings.insert(m_strings.end(),
                             br.begin + 8,  // string size is at least 8 by assumption
                             br.end);
            m_strings_offsets.push_back(m_strings.size());
        }

        void build(prefix_indexed_string_pool_v3& pool) {
            pool.m_prefixes.swap(m_prefixes);
            pool.m_pointers.swap(m_pointers);
            pool.m_strings_offsets.swap(m_strings_offsets);
            pool.m_strings.swap(m_strings);
            swap(*this);
        }

        void swap(builder& other) {
            other.m_prefixes.swap(m_prefixes);
            other.m_pointers.swap(m_pointers);
            other.m_strings_offsets.swap(m_strings_offsets);
            other.m_strings.swap(m_strings);
        }

    private:
        std::vector<uint64_t> m_prefixes;
        std::vector<pointer_type> m_pointers;
        std::vector<pointer_type> m_strings_offsets;
        std::vector<uint8_t> m_strings;
    };

    prefix_indexed_string_pool_v3() {}

    uint64_t size() const {
        assert(m_strings_offsets.size() > 0);
        return m_strings_offsets.size() - 1;
    }

    inline byte_range access(uint64_t i) const {
        assert(i < size());
        auto begin = m_strings_offsets[i];
        auto end = m_strings_offsets[i + 1];
        uint8_t const* base = reinterpret_cast<uint8_t const*>(m_strings.data());
        return {base + begin, base + end};
    }

    byte_range byte_range_from_string8(std::string const& str) const {
        const uint8_t* buf = reinterpret_cast<uint8_t const*>(str.c_str() + 8);
        const uint8_t* end = buf + str.size() - 8;  // exclude the null terminator
        return {buf, end};
    }

    uint64_t lower_bound(std::string const& val) const {
        // This first search is 5X faster than the overall process,
        // so we should not spend time in making it faster.
        // We should, instead, devise a faster solution to search
        // through a (medium-short) range of strings,
        // which is the step performed after this search.
        uint64_t x = string_to_uint<64>(val);
        auto it = std::lower_bound(m_prefixes.begin(), m_prefixes.end(), x);
        uint64_t p = std::distance(m_prefixes.begin(), it);
        uint64_t begin = m_pointers[p ? p - 1 : p];
        uint64_t end = m_pointers[p == m_prefixes.size() ? p : p + 1];
        assert(end > begin);
        int64_t count = end - begin;

        /*
            With this, we improve by 3X, thus indicating that we should optimize
            for small ranges. Build a small trie on the range?
        */
        // if (count <= 256) return count;

        /*
            Maybe we could re-use the same technique inside the range, essentially
            making a two/three-level data structure.
        */

        // option 1. small ranges are done via linear search
        // if (count < 256) {
        //     auto target = byte_range_from_string(val);
        //     while (byte_range_compare_v2(access(begin), target)) ++begin;
        //     return begin;
        // }

        // option 2. always do binary search
        // this seems to be the fastest option...
        int64_t step = 0;
        uint64_t i = begin;
        uint64_t ret = begin;
        auto target = byte_range_from_string8(val);
        while (count > 0) {
            i = ret;
            step = count / 2;
            i += step;

            /* this is faster than traditional string compare as below */
            bool less = byte_range_compare_v2(access(i), target);
            // bool less = byte_range_compare(access(i), target) < 0;

            if (less) {
                ret = ++i;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return ret;

        // option 3. cutoff to linear search
        // int64_t step = 0;
        // uint64_t i = begin;
        // uint64_t ret = begin;
        // auto target = byte_range_from_string(val);
        // while (count > 128) {
        //     i = ret;
        //     step = count / 2;
        //     i += step;
        //     bool less = byte_range_compare_v2(access(i), target);
        //     if (less) {
        //         ret = ++i;
        //         count -= step + 1;
        //     } else {
        //         count = step;
        //     }
        // }
        // while (byte_range_compare_v2(access(ret), target)) ++ret;
        // return ret;
    }

    uint64_t bytes() const {
        return m_prefixes.size() * sizeof(m_prefixes.front()) +
               m_pointers.size() * sizeof(m_pointers.front()) +
               m_strings_offsets.size() * sizeof(m_strings_offsets.front()) +
               m_strings.size() * sizeof(m_strings.front());
    }

private:
    std::vector<uint64_t> m_prefixes;
    std::vector<pointer_type> m_pointers;
    std::vector<pointer_type> m_strings_offsets;
    std::vector<uint8_t> m_strings;
};