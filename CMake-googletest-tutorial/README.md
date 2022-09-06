## Integrate Project with GoogleTest using CMake

#### What is GoogleTest and CMake?

In last section, we walk through how to write a `MakeFile` to automate the process of compiling a project. `CMake` is supposed to be a **Cross Platform** generator for `MakeFile`, so that it would work under different platforms and generate fit `MakeFile` for the project.

[**GoogleTest**](https://google.github.io/googletest) is Google's C++ testing and mocking framework. Once our project grows big, we need to write all kinds of testings to make sure the code functions as we expect. There are different types of testing: unit test, integration test, end-to-end test, etc.

In this tutorial, we aim to build a simple tool project using `CMake` and integrate with **GoogleTest**. The main source and reference is from pyk's repo [here](https://github.com/pyk/cmake-tutorial). Many thanks!

---

#### Part I: Build the Project in CMake

Firstly, install the `CMake` on your machine. Since we're on MacOS, after running `brew install cmake`, we can check the version of `CMake` is `3.24.1`:

```console
$ cmake --version
cmake version 3.24.1

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

We could have just used the command line to compile this project by

```console
$ g++ -std=c++11 -o math-lib src/main.cpp src/math.cpp
$ ./math-lib
math::add(1, 2) = 3
math::sub(4, 2) = 2
math::mul(3, 2) = 6
```

Or we could write a simple `Makefile`:

```text
CC = g++
CFLAGS = -std=c++11

all: math-lib

math-lib: src/main.cpp src/math.cpp
 	$(CC) $(CFLAGS) -o math-lib src/main.cpp src/math.cpp

.PHONY: clean
clean:
 	rm -f math-lib *.o *.s
```

and run

```console
$ make math-lib
$ ./math-lib
math::add(1, 2) = 3
math::sub(4, 2) = 2
math::mul(3, 2) = 6
$ make clean
```

However, in this project we will use an enhanced version of Makefile -- `CMake` to build the project so that it would support cross-platform compatibility.

First of all, we need to create a `CMakeLists.txt` file:

```console
$ touch CMakeLists.txt
```

and we write a very basic `CMakeLists.txt` as follows:

```console
# define the minimum required version of CMake to be used
CMAKE_MINIMUM_REQUIRED (VERSION 3.24.1)

# define the project name
PROJECT(math-lib)

# add the definition for our math lib
ADD_LIBRARY(math src/math.cpp)

# add the definition for main executable, reference the project name
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp)

# link the math library to main executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} math)
```

then we rely on `CMake` to generate the `Makefile` for us:

```console
$ mkdir build && cd build
$ cmake ..
-- The C compiler identification is AppleClang 13.0.0.13000029
-- The CXX compiler identification is AppleClang 13.0.0.13000029
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/yukunjiang/Desktop/Teach-Myself-CPP/CMake-googletest-tutorial/build

$ make math-lib
[ 25%] Building CXX object CMakeFiles/math.dir/src/math.cpp.o
[ 50%] Linking CXX static library libmath.a
[ 50%] Built target math
[ 75%] Building CXX object CMakeFiles/math-lib.dir/src/main.cpp.o
[100%] Linking CXX executable math-lib
[100%] Built target math-lib

$ ./math-lib
math::add(1, 2) = 3
math::sub(4, 2) = 2
math::mul(3, 2) = 6
```
Great! Our `CMake` is working properly and fully automated.

---

#### Part II: Integrate with GoogleTest

Now we want integrate a third-party library **GoogleTest** with our project with the `CMake`. Google actually provides a basic intoductory tutorial on how to do that [here](https://google.github.io/googletest/quickstart-cmake.html).

```console
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

# ---------- |new stuff below| --------------------

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
```

Now we will rebuild the project to add in the **GoogleTest** and run our test suite.

```console
$ cd build
$ cmake ..
-- The C compiler identification is AppleClang 13.0.0.13000029
-- The CXX compiler identification is AppleClang 13.0.0.13000029
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found Python: /Users/yukunjiang/opt/anaconda3/bin/python3.8 (found version "3.8.8") found components: Interpreter 
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/yukunjiang/Desktop/Teach-Myself-CPP/CMake-googletest-tutorial/build

$ cmake --build .
[  7%] Building CXX object CMakeFiles/math.dir/src/math.cpp.o
[ 14%] Linking CXX static library libmath.a
[ 14%] Built target math
[ 21%] Building CXX object CMakeFiles/math-lib.dir/src/main.cpp.o
[ 28%] Linking CXX executable math-lib
[ 28%] Built target math-lib
[ 35%] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[ 42%] Linking CXX static library ../../../lib/libgtest.a
[ 42%] Built target gtest
[ 50%] Building CXX object _deps/googletest-build/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[ 57%] Linking CXX static library ../../../lib/libgtest_main.a
[ 57%] Built target gtest_main
[ 64%] Building CXX object CMakeFiles/math_test.dir/test/math_test.cpp.o
[ 71%] Linking CXX executable math_test
[ 71%] Built target math_test
[ 78%] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[ 85%] Linking CXX static library ../../../lib/libgmock.a
[ 85%] Built target gmock
[ 92%] Building CXX object _deps/googletest-build/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[100%] Linking CXX static library ../../../lib/libgmock_main.a
[100%] Built target gmock_main

$ ctest
Test project /Users/yukunjiang/Desktop/Teach-Myself-CPP/CMake-googletest-tutorial/build
    Start 1: MathAddTest.EasyAddition
1/3 Test #1: MathAddTest.EasyAddition .........   Passed    0.01 sec
    Start 2: MathSubTest.EasySubtraction
2/3 Test #2: MathSubTest.EasySubtraction ......   Passed    0.00 sec
    Start 3: MathMulTest.EasyMultiplication
3/3 Test #3: MathMulTest.EasyMultiplication ...   Passed    0.01 sec

100% tests passed, 0 tests failed out of 3

Total Test time (real) =   0.02 sec
```

Done! We've successfully built and run a test binary using **GoogleTest**.
