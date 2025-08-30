# 指定最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(ThreadPoolLibrary LANGUAGES CXX)

# 设置 C++ 标准 (推荐 C++14 或更高，std::thread 在 C++11 引入)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 使用标准 C++，而非编译器扩展
set(CMAKE_BUILD_TYPE Debug)

# 查找并链接 pthread 库 (Linux/Unix 系统需要)
# 在 Windows 上通常不需要，CMake 会自动处理
find_package(Threads REQUIRED)

# 设置头文件包含目录
# 这样在代码中就可以用 #include "your_header.h" 而无需写完整路径
include_directories(include)

# 收集 src/ 目录下的所有 .cpp 源文件
# GLOB 会查找匹配模式的文件并存入变量
file(GLOB SOURCES "src/*.cpp")

# 检查是否找到了源文件
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found in 'src/' directory!")
endif()

# 创建可执行文件
add_executable(test_threadpool ${SOURCES})
add_library(threadpool SHARED ${SOURCES})

target_link_libraries(threadpool Threads::Threads)

# 将可执行文件链接到 Threads 库 (提供 pthread 支持)
target_link_libraries(test_threadpool Threads::Threads)
target_link_libraries(test_threadpool threadpool)

# (可选) 为可执行文件设置 C++ 标准
# 如果上面的全局设置不生效，可以针对目标设置



set_target_properties(test_threadpool PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# (可选) 输出一些信息，方便调试
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source files: ${SOURCES}")