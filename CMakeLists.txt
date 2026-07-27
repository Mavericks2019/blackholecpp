cmake_minimum_required(VERSION 3.5)
project(objViewer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理Qt的moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt5组件
find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

# 添加着色器资源文件
set(SHADER_RESOURCES
    shaders.qrc
 )
qt5_add_resources(RESOURCE_FILES ${SHADER_RESOURCES})

find_package(OpenMesh REQUIRED)

# 添加Eigen库支持
find_package(Eigen3 REQUIRED)

# +++ 添加CGAL依赖 +++
# 首先查找GMP和MPFR（CGAL所需）
# find_package(GMP REQUIRED)
# find_package(MPFR REQUIRED)

# 查找CGAL库
find_package(CGAL REQUIRED COMPONENTS Core)

# 启用CGAL自动链接（旧版CGAL需要，新版已移除）
if(DEFINED CGAL_USE_FILE)
  include(${CGAL_USE_FILE})
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME}
    tabs/controlpanel.h
    tabs/basiccontrolpanel.h
    tabs/multipasscontrolpanel.h
    tabs/kerrcontrolpanel.h
    glwidget/glcirclewidget.h
    glwidget/glbasicwidget.h
    glwidget/glmultipasswidget.h
    glwidget/GL2DLensingWidget.h
    glwidget/kerrwidget.h
    tabs/LensingControlPanel.h

    tabs/controlpanel.cpp
    tabs/basiccontrolpanel.cpp
    tabs/multipasscontrolpanel.cpp
    glwidget/glcirclewidget.cpp
    glwidget/glbasicwidget.cpp
    glwidget/glmultipasswidget.cpp
    glwidget/GL2DLensingWidget.cpp
    glwidget/kerrwidget.cpp
    tabs/kerrcontrolpanel.cpp
    tabs/LensingControlPanel.cpp

    mainwindow.cpp
    mainwindow.h
    main.cpp
    ${RESOURCE_FILES}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::OpenGL
    opengl32
    OpenMeshCore
    OpenMeshTools
    Eigen3::Eigen
    CGAL::CGAL
)

# 设置安装路径
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 编译后自动复制所需 DLL 到输出目录
set(QT_BIN_DIR "D:/Qt/5.11.1/msvc2017_64/bin")
set(QT_PLUGINS_DIR "D:/Qt/5.11.1/msvc2017_64/plugins")
set(VCPKG_BIN_DIR "C:/Users/Administrator/vcpkg/installed/x64-windows/bin")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${QT_BIN_DIR}/Qt5Core.dll"
    "${QT_BIN_DIR}/Qt5Gui.dll"
    "${QT_BIN_DIR}/Qt5Widgets.dll"
    "${QT_BIN_DIR}/Qt5OpenGL.dll"
    "${VCPKG_BIN_DIR}/OpenMeshCore.dll"
    "${VCPKG_BIN_DIR}/OpenMeshTools.dll"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
)