cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LibPageSub)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the library target
add_library(LibPageSub INTERFACE)

# Specify include directories for the library
target_include_directories(LibPageSub INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(LibPGSubTest ${CMAKE_CURRENT_SOURCE_DIR}/test/test_main.cpp)
target_link_libraries(LibPGSubTest LibPageSub)