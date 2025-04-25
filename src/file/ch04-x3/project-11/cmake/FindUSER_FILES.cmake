# 查找 files 头文件
find_path(FILE_INCLUDE_DIR
    NAMES file.hpp
    PATHS /include
    PATH_SUFFIXES USER_FILES
)

# 查找 files 库文件
find_library(FILE1_LIBRARY
    NAMES file1
    PATHS /lib
)

find_library(FILE2_LIBRARY
    NAMES file2
    PATHS /lib
)

# 处理 QUIET 或 REQUIRED 等参数
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(USER_FILES
    REQUIRED_VARS FILE_INCLUDE_DIR FILE1_LIBRARY FILE2_LIBRARY
)

# 设置导出变量
if(USER_FILES_FOUND)
    set(FILE_INCLUDE_DIR ${FILE_INCLUDE_DIR})
    set(FILE_LIBRARIES ${FILE1_LIBRARY} ${FILE2_LIBRARY})
endif()

# 标记为高级变量，默认隐藏
mark_as_advanced(FILE_INCLUDE_DIR FILE1_LIBRARY FILE1_LIBRARY)