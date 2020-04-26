#include <iostream>
#include <vector>
#include <random>
#include <chrono>

constexpr uint64_t LINE_SIZE = 64;

struct cache {
    cache(uint64_t s, uint64_t w)
        : size(s), ways(w), sets((size / LINE_SIZE) / ways), counters(sets, 0) {
        if (size % LINE_SIZE != 0) {
            throw std::runtime_error("cache size must be divisible by the line size");
        }
    }

    template <typename T>
    void map(T const& x) {
        uint64_t address = reinterpret_cast<uint64_t>(&x);
        uint64_t set = (address / LINE_SIZE) % sets;  // or just take log2(sets) lowest bits, after
                                                      // shifting to the right by log2(LINE_SIZE)
        counters[set] += 1;
    }

    void print_usage() const {
        std::cout << "cache specification:\n";
        std::cout << "  size " << size << "\n";
        std::cout << "  ways " << ways << "\n";
        std::cout << "  sets " << sets << "\n";
        std::cout << "cache usage:\n";
        for (uint64_t i = 0; i != sets; ++i) {
            // std::cout << "set " << i << ": " << counters[i] << "\n";
            std::cout << counters[i] << " ";
        }
        std::cout << std::endl;
    }

    void clear() {
        std::fill(counters.begin(), counters.end(), 0);
    }

    uint64_t accesses() const {
        return std::accumulate(counters.begin(), counters.end(), 0);
    }

private:
    uint64_t size;  // in bytes
    uint64_t ways;
    uint64_t sets;
    std::vector<uint64_t> counters;  // the number of addresses that are mapped to a set
};

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

struct segment_tree {
    segment_tree() : iterations(0), m_size(0) {}

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
    segment_tree_bottomup() : iterations(0), m_size(0), m_left_pos(0) {}

    template <typename T>
    void build(T const* input, uint64_t n) {
        m_size = n;
        uint64_t h = ceil(log2(n));
        uint64_t m = 2 * n - 1;
        m_tree.resize(m);
        uint64_t left_most_node_id = (uint64_t(1) << (h - 1)) - 1;
        m_left_pos = 2 * left_most_node_id + 1;
        uint64_t i = 0;
        for (; m_left_pos + i != m; ++i) m_tree[m_left_pos + i] = input[i];
        for (uint64_t j = 0; i != n; ++i, ++j) m_tree[n - 1 + j] = input[i];
        build(0);
    }

    int64_t sum(uint64_t i, cache& c) {
        uint64_t p = m_left_pos + i;
        p -= (p >= m_tree.size()) * m_size;
        int64_t sum = m_tree[p];
        c.map(m_tree[p]);
        while (p) {
            iterations += 1;
            uint64_t pp = (p - 1) / 2;
            if ((p & 1) == 0) {
                c.map(m_tree[pp]);
                sum += m_tree[pp];  // p is right child
            }
            // sum += ((p & 1) == 0) * m_tree[pp];  // p is right child
            p = pp;
        }
        return sum;
    }

    uint64_t size() const {
        return m_tree.size();
    }

    uint64_t iterations;

private:
    uint64_t m_size, m_left_pos;
    std::vector<int64_t> m_tree;

    int64_t build(uint64_t pos) {
        uint64_t left = pos * 2 + 1;
        if (left >= m_tree.size()) return m_tree[pos];  // leaf
        int64_t left_sum = build(left);
        int64_t right_sum = build(left + 1);
        m_tree[pos] = left_sum;
        return left_sum + right_sum;
    }
};

#define TEST                                                                               \
    tree.build(input.data(), input.size());                                                \
    int64_t sum = 0;                                                                       \
    auto start = clock_t::now();                                                           \
    for (auto q : queries) { sum += tree.sum(q, L1); }                                     \
    auto stop = clock_t::now();                                                            \
    auto elapsed = std::chrono::duration_cast<duration_t>(stop - start);                   \
    std::cout << "# ignore " << sum << std::endl;                                          \
    std::cout << "elapsed time: " << elapsed.count() / 1000 << " [millisec]" << std::endl; \
    std::cout << "cache usage:\n";                                                         \
    L1.print_usage();                                                                      \
    std::cout << "accesses " << L1.accesses() << std::endl;                                \
    std::cout << "iterations " << tree.iterations << std::endl;                            \
    std::cout << "size " << tree.size() << std::endl;

int main(int argc, char const** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <size>" << std::endl;
        return 1;
    }

    constexpr unsigned seed = 192390;
    std::mt19937_64 rng(seed);

    cache L1(32 * 1024, 8);  // typical parameters for L1 cache

    uint64_t n = std::stoull(argv[1]);
    if (!n) return 1;
    std::vector<int64_t> input(n, 1);

    constexpr uint64_t num_queries = 10000;
    std::uniform_int_distribution<uint64_t> distr(0, n - 1);
    std::vector<uint64_t> queries(num_queries);
    std::generate(queries.begin(), queries.end(), [&] { return distr(rng); });

    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::microseconds duration_t;

    {
        std::cout << "=== fenwick_tree\n";
        fenwick_tree tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== fenwick_tree_holes\n";
        fenwick_tree_holes tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== segment_tree\n";
        segment_tree tree;
        TEST
    }

    L1.clear();

    {
        std::cout << "=== segment_tree_bottomup\n";
        segment_tree_bottomup tree;
        TEST
    }

    return 0;
}