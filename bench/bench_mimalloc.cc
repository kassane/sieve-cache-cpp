#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>
#include <sieve.hpp>
#include <mimalloc.h>

// Custom Allocator
class MimallocMemoryResource : public std::pmr::memory_resource {
protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) final {
        return mi_malloc_aligned(bytes, alignment);
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) final {
        mi_free_size_aligned(p, bytes, alignment);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept final {
        return this == &other;
    }
};

struct S {
    S() = default;
    explicit S(const std::vector<unsigned char>& vec, unsigned long value) : a(vec), b(value) {}

    std::vector<unsigned char> a;
    unsigned long b;
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    MimallocMemoryResource mi;

    // Sequence.
    {
        SieveCache<unsigned long, unsigned long> cache(68, &mi);

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
        SieveCache<unsigned long, S> cache(68, &mi);
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
        SieveCache<unsigned long, S> cache(static_cast<size_t>(SIGMA), &mi);
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
