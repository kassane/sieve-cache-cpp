#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <sieve.hpp>
#include <thread>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_CASE("Testing SieveCache functionality") {
  SieveCache<std::string, std::string> cache(3);

  SUBCASE("Initial state checks") {
    CHECK(cache.capacity() == 3);
    CHECK(cache.empty());
  }

  SUBCASE("Insert and remove operations") {
    CHECK(cache.insert("foo", "foocontent"));
    CHECK(cache.insert("bar", "barcontent"));
    CHECK(cache.remove("bar"));
    CHECK(cache.insert("bar2", "bar2content"));
    CHECK(cache.insert("bar3", "bar3content"));
  }

  SUBCASE("Content verification after operations") {
    cache.insert("foo", "foocontent");
    cache.insert("bar", "barcontent");
    cache.remove("bar");
    cache.insert("bar2", "bar2content");
    cache.insert("bar3", "bar3content");
    CHECK(*cache.get("foo") == "foocontent");
    CHECK(cache.contains("foo"));
    CHECK(cache.get("bar") == nullptr);
    CHECK(*cache.get("bar2") == "bar2content");
    CHECK(*cache.get("bar3") == "bar3content");
    CHECK(cache.length() == 3);
  }

  SUBCASE("Clear cache and check state") {
    cache.insert("foo", "foocontent");
    cache.insert("bar", "barcontent");
    cache.remove("bar");
    cache.insert("bar2", "bar2content");
    cache.insert("bar3", "bar3content");
    cache.clear();
    CHECK(cache.length() == 0);
    CHECK(!cache.contains("foo"));
  }

  SUBCASE("Updating visited flag") {
    cache.insert("key1", "value1");
    cache.insert("key2", "value2");
    cache.insert("key1", "updated");
    cache.insert("key3", "value3");
    CHECK(cache.contains("key1"));
  }

  SUBCASE("Operator[] test") {
    cache["key1"] = "value1";
    CHECK(cache.contains("key1"));
    CHECK(*cache.get("key1") == "value1");

    cache["key1"] = "updated";
    CHECK(*cache.get("key1") == "updated");

    cache["key2"] = "value2";
    cache["key3"] = "value3";
    CHECK(cache.length() == 3);

    // Insert a new key and check for evictions
    cache["key4"] = "value4";
    CHECK(cache.length() == 3);
  }

  SUBCASE("Equality and inequality operators") {
    SieveCache<std::string, std::string> cache2(3);
    cache.insert("key1", "value1");
    cache2.insert("key1", "value1");

    CHECK(cache == cache2);

    cache2.insert("key2", "value2");
    CHECK(cache != cache2);
  }

  SUBCASE("Comparison operators") {
    SieveCache<std::string, std::string> cache2(3);
    cache.insert("key1", "value1");
    cache2.insert("key1", "value1");
    cache2.insert("key2", "value2");

    CHECK(cache < cache2);
    CHECK(cache2 > cache);
  }

  SUBCASE("Thread Safety") {
    SieveCache<int, std::string> cache3(100);

    auto insert_task = [&cache3]() {
      for (int i = 0; i < 100; ++i) {
        cache3.insert_locked(i, "value" + std::to_string(i));
      }
    };

    auto get_task = [&cache3]() {
      for (int i = 0; i < 100; ++i) {
        auto value = cache3.get_locked(i);
        if (value) {
          CHECK(*value == "value" + std::to_string(i));
        }
      }
    };

    std::thread t1(insert_task);
    std::thread t2(insert_task);
    std::thread t3(get_task);
    std::thread t4(get_task);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
  }
}
