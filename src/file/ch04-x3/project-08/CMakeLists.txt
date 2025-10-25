# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(file1)

# 添加编译选项
## 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")

# 创建静态库
add_library(file1 STATIC file1.cpp)

# 如果有头文件，设置头文件搜索路径，方便其他项目引用
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file.h")
    target_include_directories(file1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# 添加编译完成后执行命令
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -rv lib${PROJECT_NAME}.a ${CMAKE_CURRENT_SOURCE_DIR}/../project-10
    COMMAND cp -rv lib${PROJECT_NAME}.a ${CMAKE_CURRENT_SOURCE_DIR}/../project-11/library/lib
)
