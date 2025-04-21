# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(file2)

# 设备交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 创建动态库
add_library(file2 SHARED file2.cpp)

# 如果有头文件，设置头文件搜索路径，方便其他项目引用
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file.h")
    target_include_directories(file2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
