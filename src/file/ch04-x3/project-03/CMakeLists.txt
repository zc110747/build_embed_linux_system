# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

# 添加可执行文件
add_executable(target)

# 使用target_sources添加源文件
target_sources(target PRIVATE main.cpp test.cpp)

# 添加编译选项
## 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1")

# 相当于-DDDEBUG_MODE -DDEBUG_SIZE=100
target_compile_definitions(target PRIVATE DEBUG_MODE DEBUG_SIZE=100)

