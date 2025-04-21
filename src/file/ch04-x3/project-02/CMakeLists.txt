# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

# 添加可执行文件
add_executable(target main.cpp)

# 添加C++版本要求
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

