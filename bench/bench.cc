#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>
#include <sieve.hpp>

struct S {
    S() = default;
    explicit S(const std::vector<unsigned char>& vec, unsigned long value) : a(vec), b(value) {}

    std::vector<unsigned char> a;
    unsigned long b;
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    // Sequence.
    {
        SieveCache<unsigned long, unsigned long> cache(68);

        for (int i = 1; i < 1000; ++i) {
            const auto n = i % 100;
            cache[n] = n;
        }

        for (int i = 1; i < 1000; ++i) {
            const auto n = i % 100;
            cache.get(n);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed_seconds = end - start;
    std::cout << "Sequence: " << std::fixed << std::setprecision(3) << elapsed_seconds.count() << "us\n";

    // Composite.
    {
        SieveCache<unsigned long, S> cache(68);
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<unsigned long> dist(0, 99);

        for (int i = 1; i < 1000; ++i) {
            const auto n = dist(rng);
            cache[n] = S(std::vector<unsigned char>(12), n);
        }

        for (int i = 1; i < 1000; ++i) {
            const auto n = dist(rng);
            cache.get(n);
        }
    }

    end = std::chrono::high_resolution_clock::now();
    elapsed_seconds = end - start;
    std::cout << "Composite: " << std::fixed << std::setprecision(3) << elapsed_seconds.count() << "us\n";

    // CompositeNormal.
    {
        constexpr double SIGMA = 50.0 / 3.0;
        SieveCache<unsigned long, S> cache(static_cast<size_t>(SIGMA));
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<unsigned long> dist(0, 99);

        for (int i = 1; i < 1000; ++i) {
            const auto n = dist(rng);
            cache[n] = S(std::vector<unsigned char>(12), n);
        }

        for (int i = 1; i < 1000; ++i) {
            const auto n = dist(rng);
            cache.get(n);
        }
    }

    end = std::chrono::high_resolution_clock::now();
    elapsed_seconds = end - start;
    std::cout << "Composite Normal: " << std::fixed << std::setprecision(3) << elapsed_seconds.count() << "us\n";

    return 0;
}
