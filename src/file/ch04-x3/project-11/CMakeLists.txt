# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置查找库的目录(如果库不在标准目录下，需要指定路径)
set(CMAKE_PREFIX_PATH "library")

# 设置 CMake 模块路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# 查找 USER_FILES 库
find_package(USER_FILES REQUIRED)

if(USER_FILES_FOUND)
    message(STATUS "USER_FILES found: ${FILE_INCLUDE_DIR}, ${FILE_LIBRARIES}")

    add_executable(target main.cpp)

    target_include_directories(target PRIVATE ${FILE_INCLUDE_DIR})

    target_link_libraries(target PRIVATE ${FILE_LIBRARIES})
else()
    message(FATAL_ERROR "USER_FILES not found.")
endif()