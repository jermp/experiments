#include <iostream>
#include <vector>
#include <chrono>
#include <random>

/*

Blog post about cache-aliasing:
    https://pvk.ca/Blog/2012/07/30/binary-search-is-a-pathological-case-for-caches/

Blog post about (Intel) cache organization:
    https://manybutfinite.com/post/intel-cpu-caches/

*/

int main(int argc, char const** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <size>" << std::endl;
        return 1;
    }

    constexpr unsigned seed = 43581;
    std::mt19937_64 rng(seed);

    uint64_t n = std::stoull(argv[1]);
    if (!n) return 1;
    std::vector<uint64_t> vec(n, 1);

    constexpr uint64_t LINE_SIZE = 64;
    constexpr uint64_t L1_SIZE = 32 * 1024;
    constexpr uint64_t WAYS = 8;
    constexpr uint64_t PAGE_SIZE = L1_SIZE / WAYS;
    constexpr uint64_t ITEMS_INLINE = LINE_SIZE / sizeof(uint64_t);

    uint64_t stride = PAGE_SIZE / sizeof(uint64_t);
    std::cout << "stride " << stride << std::endl;

    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::microseconds duration_t;

    uint64_t accesses = n / stride;
    std::vector<uint64_t> pos(accesses);
    uint64_t sum = 0;

    {
        std::uniform_int_distribution<uint64_t> distr(0, ITEMS_INLINE - 1);
        for (uint64_t i = 0; i != accesses; ++i) {
            // accesses are always to the same set
            pos[i] = i * stride + distr(rng);
        }
        auto start = clock_t::now();
        for (int run = 0; run != 100; ++run) {
            sum = 0;
            for (auto p : pos) { sum += vec[p]; }
        }
        auto stop = clock_t::now();
        auto elapsed = std::chrono::duration_cast<duration_t>(stop - start);
        std::cout << "# ignore " << sum << std::endl;
        std::cout << "elapsed time: " << elapsed.count() / 1000 << " [millisec]" << std::endl;
    }

    {
        std::uniform_int_distribution<uint64_t> distr(0, stride - 1);
        for (uint64_t i = 0; i != accesses; ++i) {
            // accesses are evenly distributed among the sets
            pos[i] = i * stride + distr(rng);
        }
        auto start = clock_t::now();
        for (int run = 0; run != 100; ++run) {
            sum = 0;
            for (auto p : pos) { sum += vec[p]; }
        }
        auto stop = clock_t::now();
        auto elapsed = std::chrono::duration_cast<duration_t>(stop - start);
        std::cout << "# ignore " << sum << std::endl;
        std::cout << "elapsed time: " << elapsed.count() / 1000 << " [millisec]" << std::endl;
    }

    return 0;
}
