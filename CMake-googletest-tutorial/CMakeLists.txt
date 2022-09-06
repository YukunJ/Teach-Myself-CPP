# define the minimum required version of CMake to be used
CMAKE_MINIMUM_REQUIRED (VERSION 3.24.1)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 11)

# define the project name
PROJECT(math-lib)

# add the definition for our math lib
ADD_LIBRARY(math src/math.cpp)

# add the definition for main executable, reference the project name
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp)

# link the math library to main executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} math)

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

ADD_EXECUTABLE(math_test test/math_test.cpp)

# link it to GoogleTest(gtest_main)
TARGET_LINK_LIBRARIES(math_test math GTest::gtest_main)

INCLUDE(GoogleTest)

# enable CMake's test runner to discover the tests
gtest_discover_tests(math_test)