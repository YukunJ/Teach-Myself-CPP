# define the minimum required version of CMake to be used
CMAKE_MINIMUM_REQUIRED (VERSION 3.24.1)

# GoogleTest requires at least C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# define the project name
PROJECT(aba-problem)

SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

# add the definition for main executable, reference the project name
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp src/lockfree_stack_bug.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE Threads::Threads)