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
#include <mutex>
#include <shared_mutex>
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

    explicit SieveCache(size_t capacity, std::pmr::memory_resource* mem_resource)
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

    inline V& operator[](const K& key) noexcept {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return values_[i];
            }
        }
        if (size_ < capacity_) {
            construct(&keys_[size_], key);
            construct(&values_[size_]);
            ++size_;
            return values_[size_ - 1];
        }
        // Handle the case where cache is full
        return values_[0];
    }

    inline V* get(const K& key) noexcept {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return &values_[i];
            }
        }
        return nullptr;
    }

    inline V* get_locked(const K& key) noexcept {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return &values_[i];
            }
        }
        return nullptr;
    }

    constexpr size_t capacity() const {
        return capacity_;
    }

    constexpr bool empty() const {
        return size_ == 0;
    }

    inline bool insert(const K& key, const V& value) noexcept {
        if (size_ < capacity_) {
            construct(&keys_[size_], key);
            construct(&values_[size_], value);
            ++size_;
            return true;
        }
        return false; // Cache is full
    }

    inline bool insert_locked(const K& key, const V& value) noexcept {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        if (size_ < capacity_) {
            construct(&keys_[size_], key);
            construct(&values_[size_], value);
            ++size_;
            return true;
        }
        return false; // Cache is full
    }

    inline bool remove(const K& key) noexcept {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                destroy(&keys_[i]);
                destroy(&values_[i]);
                for (size_t j = i; j < size_ - 1; ++j) {
                    construct(&keys_[j], std::move(keys_[j + 1]));
                    construct(&values_[j], std::move(values_[j + 1]));
                    destroy(&keys_[j + 1]);
                    destroy(&values_[j + 1]);
                }
                --size_;
                return true;
            }
        }
        return false; // Key not found
    }

    inline bool remove_locked(const K& key) noexcept {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                destroy(&keys_[i]);
                destroy(&values_[i]);
                for (size_t j = i; j < size_ - 1; ++j) {
                    construct(&keys_[j], std::move(keys_[j + 1]));
                    construct(&values_[j], std::move(values_[j + 1]));
                    destroy(&keys_[j + 1]);
                    destroy(&values_[j + 1]);
                }
                --size_;
                return true;
            }
        }
        return false; // Key not found
    }

    inline bool contains(const K& key) const noexcept {
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return true;
            }
        }
        return false;
    }

    inline bool contains_locked(const K& key) const noexcept {
        std::lock_guard<std::shared_mutex> lock(rw_mutex_);
        for (size_t i = 0; i < size_; ++i) {
            if (keys_[i] == key) {
                return true;
            }
        }
        return false;
    }

    inline void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            keys_[i].~K();
            values_[i].~V();
        }
        size_ = 0;
    }

    inline void clear_locked() noexcept {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        for (size_t i = 0; i < size_; ++i) {
            keys_[i].~K();
            values_[i].~V();
        }
        size_ = 0;
    }

    constexpr size_t length() const noexcept {
        return size_;
    }

private:
    template<typename T, typename... Args>
    void construct(T* ptr, Args&&... args) noexcept {
        new (ptr) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void destroy(T* ptr) noexcept {
        ptr->~T();
    }
    size_t capacity_;
    size_t size_;
    K* keys_;
    V* values_;
    std::pmr::memory_resource* mem_resource_;
    mutable std::shared_mutex rw_mutex_;
};

#endif // SIEVE_HPP
