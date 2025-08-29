include(FetchContent)

FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
)
FetchContent_MakeAvailable(gtest)
