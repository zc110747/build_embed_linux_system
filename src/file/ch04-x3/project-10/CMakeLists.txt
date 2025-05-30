# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(target)

# 设备交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加可执行文件
add_executable(target main.cpp)

# 链接动态库目录
target_link_directories(target PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 添加 libpthread 和 libm
target_link_libraries(target PRIVATE pthread m)

# 添加本地动静态库
target_link_libraries(target PRIVATE file1 file2)

# 添加宏定义
target_compile_definitions(target PRIVATE DEBUG_MODE MAX_SIZE=100)

# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")