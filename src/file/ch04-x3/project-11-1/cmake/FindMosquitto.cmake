# 查找 mosquitto 头文件
find_path(Mosquitto_INCLUDE_DIR
    NAMES mosquitto.h mosquittopp.h
    PATHS /usr/include /usr/local/include /include
    PATH_SUFFIXES mosquitto
)

# 查找 mosquitto 库文件
find_library(Mosquitto_LIBRARY
    NAMES mosquitto
    PATHS /usr/lib /usr/local/lib
)

# 查找 mosquittopp 库文件（C++ 客户端）
find_library(Mosquittopp_LIBRARY
    NAMES mosquittopp
    PATHS /usr/lib /usr/local/lib
)

find_library(OpenSSL_LIBRARY
    NAMES ssl
    PATHS /usr/lib /usr/local/lib
)

find_library(Crypto_LIBRARY
    NAMES crypto
    PATHS /usr/lib /usr/local/lib
)

# 处理 QUIET 或 REQUIRED 等参数
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mosquitto
    REQUIRED_VARS Mosquitto_INCLUDE_DIR Mosquitto_LIBRARY Mosquittopp_LIBRARY OpenSSL_LIBRARY Crypto_LIBRARY
)

# 设置导出变量
if(Mosquitto_FOUND)
    set(Mosquitto_INCLUDE_DIRS ${Mosquitto_INCLUDE_DIR})
    set(Mosquitto_LIBRARIES ${Mosquitto_LIBRARY} ${Mosquittopp_LIBRARY} ${Crypto_LIBRARY} ${OpenSSL_LIBRARY})
endif()

# 标记为高级变量，默认隐藏
mark_as_advanced(Mosquitto_INCLUDE_DIR Mosquitto_LIBRARY Mosquittopp_LIBRARY OpenSSL_LIBRARY Crypto_LIBRARY)