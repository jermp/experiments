#pragma once

#include <vector>
#include <string>
#include <cassert>

#include "util.hpp"

template <uint64_t string_size>
struct fixed_string_pool {
    fixed_string_pool(uint64_t num_strings) {
        m_num_strings = num_strings;
        m_strings.reserve(num_strings * string_size);
    }

    void append(std::string const& str) {
        if (str.size() >= string_size) {
            for (uint64_t i = 0; i != string_size; ++i) m_strings.push_back(str[i]);
        } else {
            for (auto c : str) m_strings.push_back(c);
            uint64_t excess = string_size - str.size();
            for (uint64_t i = 0; i != excess; ++i) m_strings.push_back(0);
        }
    }

    uint64_t size() const {
        return m_num_strings;
    }

    byte_range access(uint64_t i) const {
        uint64_t begin = i * string_size;
        return {m_strings.data() + begin, m_strings.data() + begin + string_size};
    }

    uint64_t lower_bound(std::string const& val) const {
        int64_t count = size();
        int64_t step = 0;
        uint64_t i = 0;
        uint64_t ret = 0;
        auto target = byte_range_from_string<string_size>(val);
        while (count > 0) {
            i = ret;
            step = count / 2;
            i += step;
            int cmp = byte_range_compare(access(i), target);

            // branch-free version seems to go slower here...
            // bool flag = cmp < 0;
            // i += flag;
            // ret = flag * i + !flag * ret;
            // count = flag * (count - (step + 1)) + !flag * step;

            if (cmp < 0) {
                ret = ++i;
                count -= step + 1;
            } else {
                count = step;
            }
        }
        return ret;
    }

private:
    uint64_t m_num_strings;
    std::vector<uint8_t> m_strings;
};