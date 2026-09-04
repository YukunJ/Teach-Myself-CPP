# define the minimum required version of CMake to be used
CMAKE_MINIMUM_REQUIRED (VERSION 3.24.1)

# GoogleTest requires at least C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# define the project name
PROJECT(aba-memory-reclamation-problem)

SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

# add the definition for main executable, reference the project name
# ADD_EXECUTABLE(lockfree_stack_bug src/main.cpp src/lockfree_stack_bug.cpp)
# TARGET_COMPILE_OPTIONS(lockfree_stack_bug PRIVATE -mcx16)
# TARGET_LINK_LIBRARIES(lockfree_stack_bug PRIVATE Threads::Threads atomic)

ADD_EXECUTABLE(lockfree_stack_epoch src/main.cpp src/lockfree_stack_epoch.cpp)
TARGET_COMPILE_OPTIONS(lockfree_stack_epoch PRIVATE -mcx16)
TARGET_LINK_LIBRARIES(lockfree_stack_epoch PRIVATE Threads::Threads atomic)