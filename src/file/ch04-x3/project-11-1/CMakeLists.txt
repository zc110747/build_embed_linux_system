# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-none-linux-gnueabihf-g++)

# 设置查找库的目录(如果库不在标准目录下，需要指定路径)
set(CMAKE_PREFIX_PATH "$ENV{SUPPORT_ENV_INSTALL}")

# 设置 CMake 模块路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# 查找 Mosquitto 库
find_package(Mosquitto REQUIRED)

if(Mosquitto_FOUND)
    message(STATUS "Mosquitto found: ${Mosquitto_INCLUDE_DIRS}, ${Mosquitto_LIBRARIES}")

    add_executable(target main.cpp)

    target_include_directories(target PRIVATE ${Mosquitto_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include")

    target_link_libraries(target PRIVATE ${Mosquitto_LIBRARIES})
else()
    message(FATAL_ERROR "Mosquitto not found.")
endif()