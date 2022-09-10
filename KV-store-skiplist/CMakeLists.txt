# define the minimum required version of CMake to be used
CMAKE_MINIMUM_REQUIRED (VERSION 3.24.1)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 11)

# define the project name
PROJECT(kvstore-skiplist)

# add the definition for our math lib
# ADD_LIBRARY(skiplist src/skiplist.cpp)

# add the definition for main executable, reference the project name
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp)

# add the stress test executable
ADD_EXECUTABLE(stress_test test/stress_test.cpp)

# add a path to download an external library from github
INCLUDE(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG "main"
)

# really download the library
FetchContent_MakeAvailable(googletest)

ENABLE_TESTING()

ADD_EXECUTABLE(skiplist_test test/skiplist_test.cpp)

# link it to GoogleTest(gtest_main)
TARGET_LINK_LIBRARIES(skiplist_test GTest::gtest_main)

INCLUDE(GoogleTest)

# enable CMake's test runner to discover the tests
gtest_discover_tests(skiplist_test)