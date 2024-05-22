#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <sieve.hpp>

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
}
