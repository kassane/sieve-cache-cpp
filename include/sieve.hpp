/**
 * This is an implementation of the [SIEVE](https://cachemon.github.io/SIEVE-website)
 * cache eviction algorithm.
 * Authors: Matheus Catarino França
 * Copyright: Copyright © 2024 Matheus C. França
 * License: MIT
 */

#ifndef SIEVE_HPP
#define SIEVE_HPP

#include <cstddef>

template <typename K, typename V>
class SieveCache {
public:
    SieveCache(size_t capacity)
        : capacity_(capacity), size_(0) {
        keys_ = new K[capacity_];
        values_ = new V[capacity_];
    }

    ~SieveCache() {
        delete[] keys_;
        delete[] values_;
    }

    V& operator[](const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return values_[i];
            }
        }
        if (size_ < capacity_) {
            keys_[size_] = key;
            values_[size_] = V();
            ++size_;
            return values_[size_ - 1];
        }
        // Handle the case where cache is full
        return values_[0];
    }

    V* get(const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return &values_[i];
            }
        }
        return nullptr;
    }

    size_t capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

    bool insert(const K& key, const V& value) {
        if (size_ < capacity_) {
            keys_[size_] = key;
            values_[size_] = value;
            ++size_;
            return true;
        }
        return false; // Cache is full
    }

    bool remove(const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                // Shift elements to the left to remove the key-value pair
                for (size_t j = i; j < size_ - 1; ++j) {
                    keys_[j] = keys_[j + 1];
                    values_[j] = values_[j + 1];
                }
                --size_;
                return true;
            }
        }
        return false; // Key not found
    }

    bool contains(const K& key) const {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return true;
            }
        }
        return false;
    }

    void clear() {
        size_ = 0;
    }

    size_t length() const {
        return size_;
    }

private:
    size_t capacity_;
    size_t size_;
    K* keys_;
    V* values_;
};

#endif // SIEVE_HPP
