#pragma once

#include <vector>

struct segment_tree_topdown {
    segment_tree_topdown() : iterations(0), m_size(0) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        m_size = uint64_t(1) << static_cast<uint64_t>(ceil(log2(n)));
        m_tree.resize(2 * m_size - 1, 0);
        std::vector<int64_t> in(m_size, 0);
        std::copy(input, input + n, in.begin());
        build(in.data(), 0, m_size - 1, 0);
    }

    int64_t sum(uint64_t i, cache& c) {
        uint64_t l = 0;
        uint64_t h = m_size - 1;
        uint64_t p = 0;
        int64_t sum = 0;
        while (l < h) {
            iterations += 1;
            if (i == h) break;
            uint64_t m = (l + h) / 2;
            p = 2 * p + 1;
            if (i > m) {
                c.map(m_tree[p]);
                sum += m_tree[p];
                l = m + 1;
                p += 1;
            } else {
                h = m;
            }
        }
        c.map(m_tree[p]);
        return sum + m_tree[p];
    }

    uint64_t size() const {
        return m_tree.size();
    }

    uint64_t iterations;

private:
    uint64_t m_size;
    std::vector<int64_t> m_tree;

    int64_t build(int64_t const* input, uint64_t l, uint64_t h, uint64_t p) {
        if (l == h) return m_tree[p] = input[l];
        uint64_t m = (l + h) / 2;
        int64_t l_sum = build(input, l, m, 2 * p + 1);
        int64_t r_sum = build(input, m + 1, h, 2 * p + 2);
        return m_tree[p] = l_sum + r_sum;
    }
};

struct segment_tree_bottomup {
    segment_tree_bottomup() : iterations(0), m_size(0), m_begin(0) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        uint64_t m = 2 * n - 1;
        m_size = n;
        m_tree.resize(m);
        m_begin = (1ULL << uint64_t(ceil(log2(n)))) - 1;
        uint64_t i = 0;
        for (; m_begin + i != m; ++i) m_tree[m_begin + i] = input[i];
        for (uint64_t j = 0; i != n; ++i, ++j) m_tree[n - 1 + j] = input[i];
        build(0);
    }

    int64_t sum(uint64_t i, cache& c) {
        uint64_t p = m_begin + i;
        p -= (p >= m_tree.size()) * m_size;
        int64_t sum = m_tree[p];
        while (p) {
            iterations += 1;
            if ((p & 1) == 0) {
                c.map(m_tree[p - 1]);
                sum += m_tree[p - 1];
            }
            p = (p - 1) / 2;
        }
        return sum;
    }

    uint64_t size() const {
        return m_tree.size();
    }

    uint64_t iterations;

private:
    uint64_t m_size, m_begin;
    std::vector<int64_t> m_tree;

    int64_t build(uint64_t p) {
        if (p >= m_tree.size()) return 0;
        if (p >= m_size - 1) return m_tree[p];  // leaf
        int64_t l_sum = build(2 * p + 1);
        int64_t r_sum = build(2 * p + 2);
        return m_tree[p] = l_sum + r_sum;
    }
};