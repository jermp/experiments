#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>

typedef std::chrono::high_resolution_clock clock_type;
typedef std::chrono::microseconds duration_type;

std::vector<std::vector<uint32_t>> get_collection(const uint64_t num_vectors) {
    constexpr unsigned seed = 1234567890;
    std::mt19937_64 rng(seed);
    const uint64_t N = 100000;
    std::uniform_int_distribution<uint32_t> distr_value(0, N);
    std::uniform_int_distribution<uint32_t> distr_length(1, N / 2);
    std::vector<std::vector<uint32_t>> collection(num_vectors, std::vector<uint32_t>(0));
    for (uint64_t i = 0; i != num_vectors; ++i) {
        std::vector<uint32_t>& vec = collection[i];
        const uint64_t len = distr_length(rng);
        vec.resize(len);
        for (uint32_t& x : vec) x = distr_value(rng);
        // std::sort(vec.begin(), vec.end());
    }
    return collection;
}

void stl_operator(std::vector<uint32_t> const& vec, std::ostream& os) {
    for (auto x : vec) os << x << ' ';
    os << '\n';
}

/* Credits to Jarno Alanko for the suggestion. */
void manual_conversion(std::vector<uint32_t> const& vec, std::ostream& os) {
    static char buffer[32];
    for (uint32_t x : vec) {
        int len = 0;
        do {
            buffer[len++] = '0' + (x % 10);
            x /= 10;
        } while (x > 0);
        std::reverse(buffer, buffer + len);
        buffer[len] = ' ';
        os.write(buffer, len + 1);
    }
    os << '\n';
}

void binary(std::vector<uint32_t> const& vec, std::ostream& os) {
    os.write(reinterpret_cast<char const*>(vec.data()), vec.size() * sizeof(vec.front()));
}

template <typename WriteFunc>
void write_to_file(std::vector<std::vector<uint32_t>> const& collection,
                   std::string const& output_filename, WriteFunc f) {
    auto start = clock_type::now();
    std::ofstream out(output_filename.c_str());
    if (!out.is_open()) throw std::runtime_error("cannot open output file");
    // std::stringstream out; // similar results for std::stringstream
    for (auto const& vec : collection) f(vec, out);
    out.close();
    auto stop = clock_type::now();
    auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
    std::cout << "elapsed time: " << elapsed.count() / 1000 << " [millisec]" << std::endl;
}

int main(int argc, char const** argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <num_vectors> <alg> <output_filename>" << std::endl;
        std::cout << "Option 'alg' includes: 'operator<<', 'manual', 'binary'." << std::endl;
        return 1;
    }

    const uint64_t num_vectors = std::stoull(argv[1]);
    const std::string alg = argv[2];
    const std::string output_filename = argv[3];
    if (!num_vectors) return 1;

    auto start = clock_type::now();
    auto collection = get_collection(num_vectors);
    auto stop = clock_type::now();
    auto elapsed = std::chrono::duration_cast<duration_type>(stop - start);
    std::cout << "collection created in: " << elapsed.count() / 1000 << " [millisec]" << std::endl;

    if (alg == "operator<<") {
        write_to_file(collection, output_filename, stl_operator);
    } else if (alg == "manual") {
        write_to_file(collection, output_filename, manual_conversion);
    } else if (alg == "binary") {
        write_to_file(collection, output_filename, binary);
    } else {
        std::cerr << "unknown alg" << std::endl;
        return 1;
    }

    return 0;
}
