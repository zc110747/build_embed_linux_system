# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(target)

# 设备交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加目录
target_include_directories(event)
target_include_directories(freemodbus/include)
target_include_directories(freemodbus/ascii)
target_include_directories(freemodbus/rtu)
target_include_directories(freemodbus/tcp)
target_include_directories(port)

# 添加可执行文件

## 添加子目录的 CMakeLists.txt
add_subdirectory(freemodbus)

file(GLOB MAIN_SOURCES 
    *.cpp
    port/*.cpp)

set(ALL_SOURCES ${MAIN_SOURCES} ${FREEMODBUS_SOURCES})

add_executable(target ${ALL_SOURCES})

# 链接 pthread 和 math 库到目标可执行文件
target_link_libraries(target PRIVATE pthread m)

# 添加宏定义
target_compile_definitions(target PRIVATE DEBUG_MODE)

# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
