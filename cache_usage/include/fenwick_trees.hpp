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

    // void update(uint64_t i, int64_t delta, cache& c) {
    //     for (++i; i < size(); i += i & -i) {
    //         c.map(m_tree[i]);
    //         m_tree[i] += delta;
    //     }
    // }

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

    void update(uint64_t i, int64_t delta, cache& c) {
        for (++i; i <= m_size; i += i & -i) {
            c.map(m_tree[pos(i)]);
            m_tree[pos(i)] += delta;
        }
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
    }
};

template <typename Node>
struct fenwick_tree_blocked {
    fenwick_tree_blocked() : iterations(0), m_blocks(0), m_size(0), m_ptr(nullptr) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        m_blocks = std::ceil(static_cast<double>(n) / Node::fanout);
        m_size = n;

        std::vector<int64_t> fenwick_tree_data(m_blocks);

        auto sum = [&](uint64_t block) {
            int64_t s = fenwick_tree_data[block];
            uint64_t base = block * Node::fanout;
            for (uint64_t i = 1; i != Node::fanout; ++i) s += input[base + i];
            return s;
        };

        for (uint64_t i = 0; i != m_blocks; ++i) { fenwick_tree_data[i] = input[i * Node::fanout]; }

        for (size_t step = 2; step <= m_blocks; step *= 2) {
            for (size_t i = step - 1; i < m_blocks; i += step) {
                fenwick_tree_data[i] += sum(i - step / 2);
            }
        }

        std::vector<int64_t> node_data(Node::fanout);

        m_data.resize(pos(m_blocks * Node::bytes));
        m_ptr = m_data.data();
        uint8_t* ptr = m_data.data();
        uint64_t size = 0;
        for (uint64_t i = 0, base = 0; i != m_blocks; ++i, base += Node::fanout) {
            node_data[0] = fenwick_tree_data[i];
            for (uint64_t k = 1; k != Node::fanout and base + k < n; ++k) {
                node_data[k] = input[base + k];
            }
            Node::build(node_data.data(), ptr + pos(size));
            size += Node::bytes;
        }

        // m_data.resize(m_blocks * Node::bytes);
        // m_ptr = m_data.data();
        // uint8_t* ptr = m_data.data();
        // for (uint64_t i = 0, base = 0; i != m_blocks; ++i, base += Node::fanout) {
        //     node_data[0] = fenwick_tree_data[i];
        //     for (uint64_t k = 1; k != Node::fanout and base + k < n; ++k) {
        //         node_data[k] = input[base + k];
        //     }
        //     Node::build(node_data.data(), ptr);
        //     ptr += Node::bytes;
        // }
    }

    uint64_t size() const {
        return m_data.size() / sizeof(int64_t);
    }

    // int64_t sum(uint64_t i, cache& c) {
    //     assert(i < size());
    //     uint64_t block = i / Node::fanout + 1;
    //     uint64_t offset = i % Node::fanout;

    //     c.map(m_ptr[(block - 1) * Node::bytes]);
    //     int64_t sum = Node(m_ptr + (block - 1) * Node::bytes).sum(offset, c);
    //     while ((block &= block - 1) != 0) {
    //         c.map(m_ptr[(block - 1) * Node::bytes]);
    //         sum += Node(m_ptr + (block - 1) * Node::bytes).sum(Node::fanout - 1, c);
    //         iterations += 1;
    //     }
    //     return sum;
    // }

    int64_t sum(uint64_t i, cache& c) {
        assert(i < size());
        uint64_t block = i / Node::fanout + 1;
        uint64_t offset = i % Node::fanout;

        c.map(m_ptr[pos((block - 1) * Node::bytes)]);
        int64_t sum = Node(m_ptr + pos((block - 1) * Node::bytes)).sum(offset, c);
        while ((block &= block - 1) != 0) {
            c.map(m_ptr[pos((block - 1) * Node::bytes)]);
            sum += Node(m_ptr + pos((block - 1) * Node::bytes)).sum(Node::fanout - 1, c);
            iterations += 1;
        }
        return sum;
    }

    uint64_t iterations;

private:
    uint64_t m_blocks;
    uint64_t m_size;
    uint8_t* m_ptr;
    std::vector<uint8_t> m_data;

    static inline uint64_t pos(uint64_t i) {
        return i + (i >> 14);
    }
};
