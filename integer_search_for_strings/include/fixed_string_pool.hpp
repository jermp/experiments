#pragma once

#include <vector>
#include <string>
#include <cassert>
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
        uint64_t it = 0;
        uint64_t ret = 0;
        auto target = byte_range_from_string<string_size>(val);
        while (count > 0) {
            it = ret;
            step = count / 2;
            it += step;
            int cmp = byte_range_compare(access(it), target);
            if (cmp < 0) {
                ret = ++it;
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