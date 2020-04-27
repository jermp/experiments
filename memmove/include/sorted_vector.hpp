#pragma once

template <typename T>
struct sorted_vector {
    typedef T value_type;

    static_assert(
        std::is_pod<T>::value,
        "T must be a POD");  // not true requirement, just for simplicity and documentation

    static constexpr float grow_factor = 1.5;
    static constexpr float shrink_factor = 2.0;

    sorted_vector(uint64_t capacity = 2) {
        init(capacity);
    }

    ~sorted_vector() {
        free(m_data);
        m_size = 0;
        m_capacity = 0;
    }

    void init(uint64_t capacity) {
        m_size = 0;
        m_capacity = capacity;
        m_data = (T*)malloc(m_capacity * sizeof(T));
    }

    void insert(T const& x) {
        if (m_size == m_capacity) {
            m_capacity *= grow_factor;
            replace();
        }
        uint64_t i = rank(x);
        memmove(m_data + i + 1, m_data + i, (m_size - i) * sizeof(T));
        m_data[i] = x;
        ++m_size;
    }

    void remove(uint64_t i) {
        assert(i < m_size);
        if (m_size < m_capacity / 4) {
            m_capacity /= shrink_factor;
            replace();
        }
        --m_size;
        memmove(m_data + i, m_data + i + 1, (m_size - i) * sizeof(T));
    }

    inline uint64_t size() const {
        return m_size;
    }

    inline uint64_t capacity() const {
        return m_capacity;
    }

    struct iterator {
        iterator(T const* data, uint64_t pos = 0) : m_data(data + pos) {}

        T const& operator*() {
            return *m_data;
        }

        void operator++() {
            ++m_data;
        }

        bool operator==(iterator const& rhs) {
            return m_data == rhs.m_data;
        }

        bool operator!=(iterator const& rhs) {
            return !(*this == rhs);
        }

    private:
        T const* m_data;
    };

    iterator begin() {
        return iterator(m_data);
    }

    iterator end() {
        return iterator(m_data + m_size);
    }

    // double overhead() const {
    //     assert(capacity() >= size());
    //     return (capacity() - size()) * 100.0 / size();
    // }

private:
    uint64_t m_size;
    uint64_t m_capacity;
    T* m_data;

    void replace() {
        m_data = (T*)realloc(m_data, capacity() * sizeof(T));
    }

    uint64_t rank(T const& x) const {
        if (m_size == 0) return 0;
        uint64_t r = 0;
        uint64_t mid = m_size / 2;

        if (x > m_data[mid]) r = mid;

        // unroll 4
        while (r + 3 < m_size) {
            if (x <= m_data[r + 0]) return r + 0;
            if (x <= m_data[r + 1]) return r + 1;
            if (x <= m_data[r + 2]) return r + 2;
            if (x <= m_data[r + 3]) return r + 3;
            r += 4;
        }

        while (r != m_size and x > m_data[r]) ++r;
        return r;
    }
};
