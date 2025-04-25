# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(target)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加可执行文件
# 添加子目录的 CMakeLists.txt
add_subdirectory(lib)

file(GLOB MAIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(target)

target_sources(target PRIVATE ${MAIN_SOURCES} ${LIB_SOURCES})

target_include_directories(target PRIVATE ${INCLUDE_HEADERS})

# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
