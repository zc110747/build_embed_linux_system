# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(dynamic)

# 设备交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 创建动态库
add_library(dynamic SHARED mainso.cpp)

# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")

# 添加编译完成后执行命令
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -rv *.so ${CMAKE_CURRENT_SOURCE_DIR}/../application
    COMMAND cp -rv ../*.h ${CMAKE_CURRENT_SOURCE_DIR}/../application
)