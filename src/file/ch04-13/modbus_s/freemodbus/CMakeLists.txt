
# 收集 lib 目录下的源文件
file(GLOB FREEMODBUS_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/ascii/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/functions/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/rtu/*.c*"
        "${CMAKE_CURRENT_SOURCE_DIR}/tcp/*.c*")

# 将 LIB_SOURCES 变量传递给父级作用域
set(FREEMODBUS_SOURCES ${FREEMODBUS_SOURCES} PARENT_SCOPE)
