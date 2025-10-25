# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(target)

# 添加可执行文件
file(GLOB_RECURSE SRC_FILES 
            main.cpp
            lib/*.cpp
)
add_executable(target)

# 添加源文件
target_sources(target PRIVATE ${SRC_FILES})

# 添加头文件
target_include_directories(target PUBLIC lib)

# 添加编译选项
# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")