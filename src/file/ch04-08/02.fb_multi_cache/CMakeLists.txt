# CMakeLists.txt
# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(target)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-none-linux-gnueabihf-g++)

# set(opt_dir "/home/freedom/Desktop/Project/t113-build/tina5.0_v1.0/out/t113_i/tlt113-minievm-emmc/buildroot/buildroot/host/bin")
# set(CMAKE_C_COMPILER ${opt_dir}/arm-linux-gnueabi-gcc)
# set(CMAKE_CXX_COMPILER ${opt_dir}/arm-linux-gnueabi-g++)

# 生成可执行文件
add_executable(target main.c)

target_link_libraries(target PRIVATE m)