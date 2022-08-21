# cmake 最小要求版本
cmake_minimum_required(VERSION 3.16.6)

# mrpc 项目
project(mrpc C CXX)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_PROJECT_NAME STREQUAL "mrpc")
    option(MRPC_BUILD_EXAMPLES "Build mrpc examples" ON)
endif()

# if(CMAKE_PROJECT_NAME STREQUAL "mrpc")
#     option(MRPC_BUILD_MRPC "Build examples" ON)
# endif()

# 配置编译器
set(CMAKE_CXX_COMPILER "g++")

# 生成调试版本
# set(CMAKE_BUILD_TYPE "Debug")

set(CXX_FLAGS
    -g
    -DVALGRIND
    -O2 
    -DNDEBUG
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -DUSE_FULL_FILENAME
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpointer-arith
    -Wshadow
    -Wwrite-strings
    -march=native
    # -MMD
    -std=c++11
    -rdynamic
 )

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()

# message(${CXX_FLAGS})

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")


# 设置编译器选项
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0 -rdynamic -Wall -Wno-unused-variable")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS}" )

# 设置可执行文件输出路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 设置静态库文件输出目录
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 设置动态库文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 设置头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/net)
include_directories(${PROJECT_SOURCE_DIR}/src/base/queue)
include_directories(${PROJECT_SOURCE_DIR}/third/libco/include)
# include_directories(/usr/local/boost/include)
# include_directories(${PROJECT_SOURCE_DIR}/src/mrpc)
# include_directories(${PROJECT_SOURCE_DIR}/example/protobuf)


# 设置库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/third/libco/lib)

# src 中包含了 mprpc 框架的所有相关代码
add_subdirectory(src)
add_subdirectory(example)
# example 中包含了 mprpc 框架使用的示例代码
# add_subdirectory(example)
