cmake_minimum_required(VERSION 3.10)
project(SieveCache VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(sieve_cache source/sieve.cc)
if(MSVC)
    target_compile_options(sieve_cache PRIVATE /W4 /WX)
else()
    target_compile_options(sieve_cache PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

enable_testing()
add_test(NAME SieveCacheTest COMMAND sieve_cache)
