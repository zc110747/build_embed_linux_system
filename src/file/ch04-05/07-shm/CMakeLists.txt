# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

# 添加可执行文件
add_executable(target main.c)

target_link_libraries(target PRIVATE pthread)