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
#include <memory_resource>
#if __cplusplus >= 202002L
#include <compare>
#endif

template <typename K, typename V>
class SieveCache {
public:
    SieveCache(size_t capacity)
        : capacity_(capacity), size_(0), mem_resource_(std::pmr::get_default_resource()) {
        keys_ = static_cast<K*>(mem_resource_->allocate(capacity_ * sizeof(K)));
        values_ = static_cast<V*>(mem_resource_->allocate(capacity_ * sizeof(V)));
    }

    SieveCache(size_t capacity, std::pmr::memory_resource* mem_resource)
        : capacity_(capacity), size_(0), mem_resource_(mem_resource) {
        keys_ = static_cast<K*>(mem_resource_->allocate(capacity_ * sizeof(K)));
        values_ = static_cast<V*>(mem_resource_->allocate(capacity_ * sizeof(V)));
    }

    ~SieveCache() {
        for (size_t i = 0; i < size_; ++i) {
            keys_[i].~K();
            values_[i].~V();
        }
        mem_resource_->deallocate(keys_, capacity_ * sizeof(K));
        mem_resource_->deallocate(values_, capacity_ * sizeof(V));
    }

#if __cplusplus >= 202002L && __has_include(<compare>)
    auto operator<=>(const SieveCache<K, V>& other) const {
        if (size_ <=> other.size_ != 0) {
            return size_ <=> other.size_;
        }
        for (size_t i = 0; i < size_; ++i) {
            if (auto cmp = keys_[i] <=> other.keys_[i]; cmp != 0) {
                return cmp;
            }
            if (auto cmp = values_[i] <=> other.values_[i]; cmp != 0) {
                return cmp;
            }
        }
        return std::strong_ordering::equal;
    }
#else
    bool operator<(const SieveCache& other) const {
        return size_ < other.size_;
    }

    bool operator>(const SieveCache& other) const {
        return size_ > other.size_;
    }
#endif
    bool operator==(const SieveCache& other) const {
        if (size_ != other.size_ || capacity_ != other.capacity_) {
            return false;
        }
        for (size_t i = 0; i < size_; ++i) {
            if (!(keys_[i] == other.keys_[i] && values_[i] == other.values_[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SieveCache& other) const {
        return !(*this == other);
    }

    V& operator[](const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return values_[i];
            }
        }
        if (size_ < capacity_) {
            new (&keys_[size_]) K(key);
            new (&values_[size_]) V();
            ++size_;
            return values_[size_ - 1];
        }
        return values_[0]; // Simplified eviction policy
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
            new (&keys_[size_]) K(key);
            new (&values_[size_]) V(value);
            ++size_;
            return true;
        }
        return false; // Cache is full
    }

    bool remove(const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                keys_[i].~K();
                values_[i].~V();
                for (size_t j = i; j < size_ - 1; ++j) {
                    new (&keys_[j]) K(std::move(keys_[j + 1]));
                    new (&values_[j]) V(std::move(values_[j + 1]));
                    keys_[j + 1].~K();
                    values_[j + 1].~V();
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
        for (size_t i = 0; i < size_; ++i) {
            keys_[i].~K();
            values_[i].~V();
        }
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
    std::pmr::memory_resource* mem_resource_;
};

#endif // SIEVE_HPP
