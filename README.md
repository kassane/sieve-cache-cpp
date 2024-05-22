# Sieve Cache in C++ (header only)

A [SIEVE cache](https://cachemon.github.io/SIEVE-website/) implementation for C++. Based on [D implementation](https://github.com/kubo39/sieve-cache-d).

### How to Use

* Test on [godbolt](https://godbolt.org/z/fej9cM61j)

**or**

* Add in your `CMakeLists.txt`

```cmake
include(FetchContent)
FetchContent_Declare(
    sieve
    GIT_REPOSITORY "https://github.com/kassane/sieve-cache-cpp.git"
    GIT_TAG main
)
set(BUILD_BENCH OFF)
set(BUILD_TESTS OFF)
FetchContent_GetProperties(sieve)
if(NOT sieve_POPULATED)
    FetchContent_Populate(sieve)
endif()
set(SIEVE_PATH "${sieve_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME} "demo.cc")
target_include_directories(${PROJECT_NAME} PRIVATE ${SIEVE_PATH})
```

```bash
$ cmake -B build
$ cmake --build build
# Run benchmark sample (if enabled)
$ cmake --build build --target bench
# Run unittest  (if enabled)
$ cmake --build build --target unittest
```

## LICENSE

See: [LICENSE](LICENSE)
