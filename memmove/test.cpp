#include <iostream>
#include <vector>
#include <chrono>
#include <set>
#include <algorithm>
#include <numeric>
#include <random>
#include <iomanip>

#include "include/sorted_vector.hpp"

template <typename Set>
void test() {
    typedef typename Set::value_type value_t;
    constexpr int MAX_SIZE = 4000;
    constexpr int width = 10;
    constexpr unsigned seed = 828371;
    constexpr int runs = 100;  // must be more than 2
    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::microseconds duration_t;

    std::vector<double> timings(runs);
    std::vector<value_t> insertions;
    std::mt19937_64 rng(seed);

    std::cout << std::right << std::setw(width) << "SIZE" << std::setw(width) << "MIN"
              << std::setw(width) << "AVG" << std::setw(width) << "MAX" << std::endl;
    std::cout << std::right << std::setw(width) << "----" << std::setw(width) << "---"
              << std::setw(width) << "---" << std::setw(width) << "---" << std::endl;

    for (uint64_t size = 10; size <= MAX_SIZE; size += 10) {
        insertions.resize(size);
        uint64_t x = 0;
        std::generate(insertions.begin(), insertions.end(), [&] { return x++; });
        std::shuffle(insertions.begin(), insertions.end(), rng);

        for (int run = 0; run != runs; ++run) {
            Set s;
            auto start = clock_t::now();
            for (auto x : insertions) { s.insert(x); }
            auto stop = clock_t::now();
            auto elapsed = std::chrono::duration_cast<duration_t>(stop - start);
            timings[run] = elapsed.count();
        }

        std::sort(timings.begin(), timings.end());
        double avg = std::accumulate(timings.begin() + 1, timings.end() - 1, 0.0) / (runs - 2);
        auto ns_per_op = [&](double time) { return ceil(time / size * 1000); };
        std::cout << std::right << std::setw(width) << size << std::setw(width)
                  << ns_per_op(timings.front()) << std::setw(width) << ns_per_op(avg)
                  << std::setw(width) << ns_per_op(timings.back()) << std::endl;
    }
}

int main() {
    test<sorted_vector<int>>();
    test<std::set<int>>();
    // TODO: add rotated vectors from https://www.ics.uci.edu/~goodrich/pubs/wads99.pdf

    return 0;
}