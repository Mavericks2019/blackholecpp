cmake_minimum_required(VERSION 3.5)

# 设置项目名称和语言
project(OpenGLTest LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型为 Release（如果没有指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 查找 Qt5 组件
find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)

# 创建可执行文件
add_executable(opengl_test minimal_opengl_test.cpp)

# 链接 Qt5 库
target_link_libraries(opengl_test
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    GL
)

# 设置目标属性
target_compile_definitions(opengl_test PRIVATE QT_NO_KEYWORDS)