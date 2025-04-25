# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

# 添加可执行文件
add_executable(target)

target_sources(target PRIVATE main.cpp)

# 添加编译选项
## 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1")

target_link_libraries(target PRIVATE mosquitto mosquittopp)
