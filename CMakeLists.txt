cmake_minimum_required(VERSION 3.1)
project(tiny_muduo)

# 设置 C++ 标准为 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# muduo 最终编译成so动态库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -pthread")

# 定义参与编译的源文件
aux_source_directory(. SRC_LIST)

# 编译生成动态库
add_library(tiny_muduo SHARED ${SRC_LIST})