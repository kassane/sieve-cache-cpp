if(BUILD_BENCH)
    FetchContent_Declare(
        mmaloc
        GIT_REPOSITORY "https://github.com/microsoft/mimalloc.git"
        GIT_TAG v2.1.6
    )
    set(MI_BUILD_SHARED OFF)
    set(MI_BUILD_OBJECT OFF)
    set(MI_BUILD_TESTS OFF)
    set(MI_SECURE OFF)
    set(MI_XMALLOC OFF)
    FetchContent_GetProperties(mmaloc)
    FetchContent_MakeAvailable(mmaloc)
endif()
