cmake_minimum_required(VERSION 3.8...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(SieveCache VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose default type of build (Debug)" FORCE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
elseif(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CONFIGURATION_TYPES "Release")
else()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_BENCH "Build bench" ON)
option(BUILD_TESTS "Build tests" ON)
include(cmake/doctest.cmake)
include(cmake/mimalloc.cmake)

if(BUILD_TESTS)
    add_executable(${PROJECT_NAME}_test "tests/test.cc")
    target_include_directories(${PROJECT_NAME}_test PUBLIC "include")
    target_link_libraries(${PROJECT_NAME}_test PRIVATE doctest::doctest)

    if(MSVC)
        target_compile_options(${PROJECT_NAME}_test PRIVATE /W4 /WX)
    else()
        target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()

    add_custom_target(unittest
        COMMAND ${PROJECT_NAME}_test
        DEPENDS ${PROJECT_NAME}_test
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
if(BUILD_BENCH)
    set(BENCHMARK_SOURCES
        "bench/bench.cc"
        "bench/bench_mimalloc.cc"
    )

    foreach(benchmark_source ${BENCHMARK_SOURCES})
        get_filename_component(benchmark_name ${benchmark_source} NAME_WE)
        add_executable(${PROJECT_NAME}_${benchmark_name} ${benchmark_source})
        target_include_directories(${PROJECT_NAME}_${benchmark_name} PUBLIC "include")

        if(${benchmark_source} STREQUAL "bench/bench_mimalloc.cc")
            target_link_libraries(${PROJECT_NAME}_${benchmark_name} PUBLIC mimalloc-static)
        endif()

        if(MSVC)
            target_compile_options(${PROJECT_NAME}_${benchmark_name} PRIVATE /W4 /WX)
        else()
            target_compile_options(${PROJECT_NAME}_${benchmark_name} PRIVATE -Wall -Wextra -pedantic -Werror)
        endif()

        add_custom_target(${benchmark_name}
            COMMAND ${PROJECT_NAME}_${benchmark_name}
            DEPENDS ${PROJECT_NAME}_${benchmark_name}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endforeach()
endif()
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
install(DIRECTORY include DESTINATION include)
