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
    # -O2 
    # -DNDEBUG
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
    -std=c++17
    -rdynamic
 )

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()

# message(${CXX_FLAGS})

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# message(${CXX_FLAGS})

# message(${CXX_FLAGS})

# set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# # -O2 assert 断言会被优化掉
# set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

find_package(Boost REQUIRED)
find_package(Protobuf)
find_package(CURL)
find_package(ZLIB)
find_path(CARES_INCLUDE_DIR ares.h)
find_library(CARES_LIBRARY NAMES cares)
find_path(MHD_INCLUDE_DIR microhttpd.h)
find_library(MHD_LIBRARY NAMES microhttpd)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
find_library(BOOSTSYSTEM_LIBRARY NAMES boost_system)
find_path(TCMALLOC_INCLUDE_DIR gperftools/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc_and_profiler)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIBRARY NAMES hiredis)
find_path(GD_INCLUDE_DIR gd.h)
find_library(GD_LIBRARY NAMES gd)
find_program(THRIFT_COMPILER thrift)
find_path(THRIFT_INCLUDE_DIR thrift)
find_library(THRIFT_LIBRARY NAMES thrift)

if(CARES_INCLUDE_DIR AND CARES_LIBRARY)
  message(STATUS "found cares")
endif()
if(CURL_FOUND)
  message(STATUS "found curl")
endif()
if(PROTOBUF_FOUND)
  message(STATUS "found protobuf")
endif()
if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  message(STATUS "found tcmalloc")
endif()
if(ZLIB_FOUND)
  message(STATUS "found zlib")
endif()
if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
  message(STATUS "found hiredis")
endif()
if(GD_INCLUDE_DIR AND GD_LIBRARY)
  message(STATUS "found gd")
endif()
if(THRIFT_COMPILER AND THRIFT_INCLUDE_DIR AND THRIFT_LIBRARY)
  message(STATUS "found thrift")
endif()

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

# example 中包含了 mprpc 框架使用的示例代码
# add_subdirectory(example)
