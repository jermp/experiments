#pragma once

#include <vector>
#include <cassert>
#include <cmath>

#include "util.hpp"
#include "prefix_indexed_string_pool.hpp"

template <uint64_t BucketSize>
struct prefix_indexed_front_coded_dictionary {
    struct builder {
        builder() {}

        template <typename Iterator>
        void build(Iterator begin, uint64_t n) {
            m_size = n;
            uint64_t buckets = std::ceil(static_cast<double>(n) / (BucketSize + 1));
            uint64_t tail = n - ((n / (BucketSize + 1)) * (BucketSize + 1));
            if (tail) tail -= 1;  // remove header

            std::cout << "n " << n << std::endl;
            std::cout << "buckets " << buckets << std::endl;

            m_buckets_offsets.reserve(buckets + 1);
            m_buckets_offsets.push_back(0);

            std::string prev, curr, header;
            for (uint64_t b = 0; b != buckets; ++b) {
                header = *begin++;
                headers.push_back(header);
                prev.swap(header);
                uint64_t size = b != buckets - 1 ? BucketSize : tail;
                for (uint64_t i = 0; i != size; ++i) {
                    curr = *begin++;
                    uint64_t l = 0;  // |lcp(curr,prev)|
                    while (l != curr.size() and l != prev.size() and curr[l] == prev[l]) { ++l; }
                    assert(l < 256);
                    m_data.push_back(l);
                    uint64_t size = curr.size();
                    assert(size >= l);
                    m_data.push_back(size - l);
                    m_data.insert(m_data.end(), curr.begin() + l, curr.end());
                    prev.swap(curr);
                }
                static const uint64_t max_addressable_size = uint64_t(1) << 32;
                if (m_data.size() >= max_addressable_size) {
                    throw std::runtime_error(
                        "Error: offsets to buckets must be made 64-bit "
                        "integers");
                }
                m_buckets_offsets.push_back(m_data.size());
            }

            // NOTE: pad to allow fixed-copy operations
            for (uint64_t i = 0; i != constants::max_string_length - 1; ++i) {
                m_data.push_back(0);
            }

            std::cout << "DONE" << std::endl;

            std::cout << "headers.size() " << headers.size() << std::endl;
        }

        void swap(builder& other) {
            std::swap(other.m_size, m_size);
            other.headers.swap(headers);
            other.m_buckets_offsets.swap(m_buckets_offsets);
            other.m_data.swap(m_data);
        }

        void build(prefix_indexed_front_coded_dictionary& dict) {
            dict.m_size = m_size;

            prefix_indexed_string_pool::builder prefixes_builder(headers.size());
            prefixes_builder.build(headers.begin(), headers.size());
            prefixes_builder.build(dict.m_pool);

            dict.m_buckets_offsets.swap(m_buckets_offsets);
            dict.m_data.swap(m_data);
            builder().swap(*this);
        }

    private:
        uint64_t m_size;
        std::vector<std::string> headers;
        std::vector<uint32_t> m_buckets_offsets;
        std::vector<uint8_t> m_data;
    };

    template <typename Visitor>
    void visit(Visitor& visitor) {
        visitor.visit(m_size);
        visitor.visit(m_pool);
        visitor.visit(m_buckets_offsets);
        visitor.visit(m_data);
    }

    uint64_t size() const {
        return m_size;
    }

    uint64_t lookup(byte_range string) const {
        auto [header, string_is_header, bucket] = locate_bucket(string);
        uint64_t base = bucket * (BucketSize + 1);
        if (string_is_header) return base;
        uint64_t offset = lookup(string, header, bucket);
        if (offset == constants::invalid_id) return constants::invalid_id;
        return base + offset;
    }

    uint64_t lower_bound(byte_range string) const {
        auto [header, string_is_header, bucket] = locate_bucket(string);
        uint64_t base = bucket * (BucketSize + 1);
        if (string_is_header) return base;
        uint64_t offset = lower_bound(string, header, bucket);
        if (offset == constants::invalid_id) return constants::invalid_id;
        return base + offset;
    }

    uint64_t access(uint64_t id, uint8_t* string) const {
        assert(id < size());
        uint64_t bucket = id / (BucketSize + 1);
        uint64_t offset = id % (BucketSize + 1);
        return access(bucket, offset, string);
    }

    std::string access(uint64_t id) const {
        std::string string;
        string.resize(2 * constants::max_string_length);
        uint64_t size = access(id, reinterpret_cast<uint8_t*>(string.data()));
        string.resize(size);
        return string;
    }

private:
    uint64_t m_size;
    prefix_indexed_string_pool m_pool;
    std::vector<uint32_t> m_buckets_offsets;
    std::vector<uint8_t> m_data;

    uint64_t buckets() const {
        return m_pool.size();
    }

    uint64_t bucket_size(uint64_t bucket) const {
        if (bucket != buckets() - 1) return BucketSize;
        uint64_t tail = size() - ((size() / (BucketSize + 1)) * (BucketSize + 1));
        if (tail) tail -= 1;
        return tail;
    }

    std::tuple<byte_range, bool, int> locate_bucket(byte_range string) const {
        uint64_t p = m_pool.lower_bound(string);
        return {m_pool.access(p), false, p};  // TODO: fixme

        // int lo = p, hi = buckets() - 1 - p, mi = 0, cmp = 0;
        // byte_range header;
        // int bucket;
        // while (lo <= hi) {
        //     mi = (lo + hi) / 2;
        //     header = access_header(mi);
        //     cmp = byte_range_compare(header, string);
        //     if (cmp > 0) {
        //         hi = mi - 1;
        //     } else if (cmp < 0) {
        //         lo = mi + 1;
        //     } else {
        //         bucket = mi;
        //         return {header, true, bucket};
        //     }
        // }
        // if (cmp < 0) {
        //     bucket = mi;
        // } else {
        //     bucket = hi == -1 ? 0 : hi;
        //     header = access_header(bucket);
        // }
        // return {header, false, bucket};
    }

    uint8_t decode(uint8_t const* in, uint8_t* out, uint8_t* lcp_len) const {
        *lcp_len = *in++;
        uint8_t l = *lcp_len;
        uint8_t suffix_len = *in++;
        memcpy(out + l, in, constants::max_string_length);
        return l + suffix_len;
    }

    uint64_t lower_bound(byte_range string, byte_range header, uint64_t bucket) const {
        static uint8_t decoded[2 * constants::max_string_length];
        memcpy(decoded, header.begin, constants::max_string_length);
        uint64_t n = bucket_size(bucket);
        uint8_t lcp_len;
        uint8_t const* curr = m_data.data() + m_buckets_offsets[bucket];
        for (uint64_t i = 0; i != n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare(string, {decoded, decoded + l});
            if (cmp <= 0) return i + 1;
            curr += l - lcp_len + 2;
        }
        return constants::invalid_id;
    }

    uint64_t lookup(byte_range string, byte_range header, uint64_t bucket) const {
        static uint8_t decoded[2 * constants::max_string_length];
        memcpy(decoded, header.begin, constants::max_string_length);
        uint64_t n = bucket_size(bucket);
        uint8_t lcp_len;
        uint8_t const* curr = m_data.data() + m_buckets_offsets[bucket];
        for (uint64_t i = 0; i != n; ++i) {
            uint8_t l = decode(curr, decoded, &lcp_len);
            int cmp = byte_range_compare(string, {decoded, decoded + l});
            if (cmp == 0) return i + 1;
            if (cmp < 0) return constants::invalid_id;
            curr += l - lcp_len + 2;
        }
        return constants::invalid_id;
    }

    uint64_t access(uint64_t bucket, uint64_t id, uint8_t* string) const {
        assert(id <= bucket_size(bucket));
        byte_range header = m_pool.access(bucket);
        memcpy(string, header.begin, constants::max_string_length);
        uint8_t lcp_len;
        uint8_t const* curr = m_data.data() + m_buckets_offsets[bucket];
        uint64_t size = header.end - header.begin;
        for (uint64_t i = 1; i <= id; ++i) {
            size = decode(curr, string, &lcp_len);
            curr += size - lcp_len + 2;
        }
        return size;
    }
};