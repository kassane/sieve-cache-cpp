/**
 * This is an implementation of the [SIEVE](https://cachemon.github.io/SIEVE-website)
 * cache eviction algorithm.
 * Authors: Matheus Catarino França
 * Copyright: Copyright © 2024 Matheus C. França
 * License: MIT
 */

#include <unordered_map>
#include <memory>
#include <cassert>
#include <string>
#include <atomic>

template <typename K, typename V>
class SieveCache {
public:
    explicit SieveCache(size_t capacity) 
        : capacity_(capacity), head_(nullptr), tail_(nullptr), hand_(nullptr), length_(0) 
    {
        assert(capacity > 0 && "capacity must be greater than zero.");
    }

    size_t capacity() const {
        return capacity_;
    }

    size_t length() const {
        return length_.load();
    }

    bool empty() const {
        return length_.load() == 0;
    }

    bool contains(const K& key) const {
        return cache_.find(key) != cache_.end();
    }

    std::shared_ptr<V> get(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return nullptr;
        }
        it->second->visited = true;
        return std::make_shared<V>(it->second->value);
    }

    bool insert(const K& key, const V& value) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second->value = value;
            it->second->visited = true;
            return false;
        }
        if (length_.load() >= capacity_) {
            evict();
        }
        auto node = std::make_shared<Node>(key, value);
        addNode(node);
        cache_[key] = node;
        length_++;
        return true;
    }

    bool remove(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) {
            return false;
        }
        auto node = it->second;
        if (node == hand_) {
            hand_ = node->prev.lock();
        }
        removeNode(node);
        cache_.erase(it);
        length_--;
        return true;
    }

    void clear() {
        cache_.clear();
        head_.reset();
        tail_.reset();
        hand_.reset();
        length_ = 0;
    }

private:
    struct Node {
        K key;
        V value;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;
        bool visited;

        Node(const K& k, const V& v) : key(k), value(v), visited(false) {}
    };

    void addNode(std::shared_ptr<Node> node) {
        node->next = head_;
        node->prev.reset();
        if (head_) {
            head_->prev = node;
        }
        head_ = node;
        if (!tail_) {
            tail_ = head_;
        }
    }

    void removeNode(std::shared_ptr<Node> node) {
        auto prev = node->prev.lock();
        auto next = node->next;

        if (prev) {
            prev->next = next;
        } else {
            head_ = next;
        }

        if (next) {
            next->prev = prev;
        } else {
            tail_ = prev;
        }

        node->prev.reset();
        node->next.reset();
    }

    void evict() {
        auto node = hand_ ? hand_ : tail_;
        while (node) {
            if (!node->visited) {
                break;
            }
            node->visited = false;
            node = node->prev.lock() ? node->prev.lock() : tail_;
        }

        if (node) {
            hand_ = node->prev.lock();
            cache_.erase(node->key);
            removeNode(node);
            length_--;
        }
    }

    size_t capacity_;
    std::shared_ptr<Node> head_;
    std::shared_ptr<Node> tail_;
    std::shared_ptr<Node> hand_;
    std::atomic<size_t> length_;
    std::unordered_map<K, std::shared_ptr<Node>> cache_;
};

int main() {
    SieveCache<std::string, std::string> cache(3);
    assert(cache.capacity() == 3);
    assert(cache.empty());
    assert(cache.insert("foo", "foocontent"));
    assert(cache.insert("bar", "barcontent"));
    assert(cache.remove("bar"));
    assert(cache.insert("bar2", "bar2content"));
    assert(cache.insert("bar3", "bar3content"));
    assert(*cache.get("foo") == "foocontent");
    assert(cache.contains("foo"));
    assert(cache.get("bar") == nullptr);
    assert(*cache.get("bar2") == "bar2content");
    assert(*cache.get("bar3") == "bar3content");
    assert(cache.length() == 3);
    cache.clear();
    assert(cache.length() == 0);
    assert(!cache.contains("foo"));

    // Additional test for updating visited flag
    cache.insert("key1", "value1");
    cache.insert("key2", "value2");
    cache.insert("key1", "updated");
    cache.insert("key3", "value3");
    assert(cache.contains("key1"));
    return 0;
}
