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

# 添加自定义命令
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Checking for required libraries..."
    COMMAND ${CMAKE_COMMAND} -E echo "Checking libfile1.a"
    COMMAND test -f "${CMAKE_CURRENT_SOURCE_DIR}/library/lib/libfile1.a" || (echo "libfile1.so not found, please build project-08" && exit 1)
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Checking libfile2.so"
    COMMAND test -f "${CMAKE_CURRENT_SOURCE_DIR}/library/lib/libfile2.so" || (echo "libfile2.so not found, please build project-09" && exit 1)
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
