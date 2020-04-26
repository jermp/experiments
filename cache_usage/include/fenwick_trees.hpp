#pragma once

#include <vector>

struct fenwick_tree {
    fenwick_tree() : iterations(0), m_size(0) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        m_size = n;
        m_tree.resize(n + 1, 0);
        for (size_t i = 1; i <= n; i++) m_tree[i] = input[i - 1];
        for (size_t step = 2; step <= n; step *= 2) {
            for (size_t i = step; i <= n; i += step) m_tree[i] += m_tree[i - step / 2];
        }
    }

    int64_t sum(uint64_t i, cache& c) {
        int64_t sum = 0;
        for (++i; i != 0; i &= i - 1) {
            c.map(m_tree[i]);
            iterations += 1;
            sum += m_tree[i];
        }
        return sum;
    }

    uint64_t size() const {
        return m_tree.size();
    }

    uint64_t iterations;

private:
    uint64_t m_size;
    std::vector<int64_t> m_tree;
};

struct fenwick_tree_holes {
    fenwick_tree_holes() : iterations(0), m_size(0) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        m_size = n;
        m_tree.resize(pos(n) + 1, 0);
        for (size_t i = 1; i <= n; i++) m_tree[pos(i)] = input[i - 1];
        for (size_t step = 2; step <= n; step *= 2) {
            for (size_t i = step; i <= n; i += step) m_tree[pos(i)] += m_tree[pos(i - step / 2)];
        }
    }

    int64_t sum(uint64_t i, cache& c) {
        int64_t sum = 0;
        for (++i; i != 0; i &= i - 1) {
            c.map(m_tree[pos(i)]);
            iterations += 1;
            sum += m_tree[pos(i)];
        }
        return sum;
    }

    uint64_t size() const {
        return m_tree.size();
    }

    uint64_t iterations;

private:
    uint64_t m_size;
    std::vector<int64_t> m_tree;

    static inline uint64_t pos(uint64_t i) {
        return i + (i >> 14);
        // constexpr uint64_t divisor = (64 + 1) * 267;
        // return i + i / divisor;
    }
};