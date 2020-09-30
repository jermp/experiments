#pragma once

#include <vector>
#include <string>
#include <cassert>

#include "util.hpp"

/* A pool of strings. Differently from a std::vector<std::string>, this class
uses a contiguous chunk of memory and uses integer pointers to keep track of
where each individual string begins (and ends). It also avoids the null terminator '\0'. */

struct string_pool {
    typedef uint32_t pointer_type;

    struct builder {
        builder(uint64_t num_strings = 0) {
            m_endpoints.reserve(num_strings + 1);
            m_endpoints.push_back(0);
        }

        template <typename Iterator>
        void build(Iterator begin, uint64_t n) {
            for (uint64_t i = 0; i != n; ++i, ++begin) {
                append(byte_range_from_string(*begin));
                if (m_strings.size() > (uint64_t(1) << (sizeof(pointer_type) * 8))) {
                    throw std::runtime_error(std::to_string(sizeof(pointer_type) * 8) +
                                             " bits per pointers are not enough");
                }
            }
        }

        void append(byte_range br) {
            m_strings.insert(m_strings.end(), br.begin, br.end);
            m_endpoints.push_back(m_strings.size());
        }

        void build(string_pool& pool) {
            pool.m_endpoints.swap(m_endpoints);
            pool.m_strings.swap(m_strings);
            swap(*this);
        }

        void swap(builder& other) {
            other.m_endpoints.swap(m_endpoints);
            other.m_strings.swap(m_strings);
        }

    private:
        std::vector<pointer_type> m_endpoints;
        std::vector<uint8_t> m_strings;
    };

    string_pool() {}

    uint64_t size() const {
        assert(m_endpoints.size() > 0);
        return m_endpoints.size() - 1;
    }

    byte_range access(uint64_t i) const {
        assert(i < size());
        auto begin = m_endpoints[i];
        auto end = m_endpoints[i + 1];
        uint8_t const* base = reinterpret_cast<uint8_t const*>(m_strings.data());
        return {base + begin, base + end};
    }

    uint64_t lower_bound(std::string const& val) const {
        int64_t count = size();
        int64_t step = 0;
        uint64_t i = 0;
        uint64_t ret = 0;
        auto target = byte_range_from_string(val);
        while (count > 0) {
            i = ret;
            step = count / 2;
            i += step;
            int cmp = byte_range_compare(access(i), target);
            if (cmp < 0) {
                ret = ++i;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return ret;
    }

    uint64_t bytes() const {
        return m_endpoints.size() * sizeof(m_endpoints.front()) +
               m_strings.size() * sizeof(m_strings.front());
    }

private:
    std::vector<pointer_type> m_endpoints;
    std::vector<uint8_t> m_strings;
};