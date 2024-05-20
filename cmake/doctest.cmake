include(FetchContent)
if(BUILD_TESTS)
    FetchContent_Declare(
        doc
        GIT_REPOSITORY "https://github.com/doctest/doctest.git"
        GIT_TAG v2.4.11
    )
    FetchContent_GetProperties(doc)
    FetchContent_MakeAvailable(doc)
endif()
