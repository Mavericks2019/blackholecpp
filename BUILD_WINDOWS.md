# Black Hole Viewer — Windows 编译指南

## 环境依赖

| 依赖 | 版本 | 路径 |
|------|------|------|
| Visual Studio | 2022 Community | `C:\Program Files\Microsoft Visual Studio\2022\Community` |
| CMake | ≥ 3.10 | `D:\cmake4.2\bin` |
| Qt 5 | 5.11.1 (MSVC 2017) | `D:\Qt\5.11.1\msvc2017_64` |
| vcpkg 库 | x64-windows | `C:\Users\Administrator\vcpkg` |

### vcpkg 已安装的库（无需手动安装）

- **OpenMesh** — 网格几何处理
- **Eigen3** — 线性代数
- **CGAL** (header-only) — 计算几何
- **Boost** — CGAL 依赖
- **GMP / MPFR** — 高精度数学（CGAL Core 依赖）

```bash
# 如果缺少某个包，可以单独安装：
vcpkg install openmesh:x64-windows
vcpkg install eigen3:x64-windows
vcpkg install cgal:x64-windows
```

## 快速开始

### 命令行编译

```cmd
cd D:\blackholecpp
mkdir build
cd build

:: 配置（自动生成 objViewer.sln）
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="C:/Users/Administrator/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
  -DCMAKE_PREFIX_PATH="D:/Qt/5.11.1/msvc2017_64"

:: 编译 Release
cmake --build . --config Release
```

产物：`build\Release\objViewer.exe`

### VS2022 打开方式

#### 方式一：打开 .sln（传统）

CMake 配置后会自动生成 `build\objViewer.sln`，直接双击即可。

> VS 内部运行需设置环境变量：
> 右键 `objViewer` 项目 → 属性 → 调试 → 环境 →
> ```
> PATH=D:\Qt\5.11.1\msvc2017_64\bin;C:\Users\Administrator\vcpkg\installed\x64-windows\bin;%PATH%
> ```

#### 方式二：CMake 集成（推荐）

1. VS2022 → 文件 → 打开 → CMake… → 选择 `CMakeLists.txt`
2. 顶部工具栏下拉选择预设：**"Windows x64 (VS 2022)"**
3. 下拉选配置：**Release**
4. 启动项选：**objViewer.exe**（不是 ALL_BUILD）
5. `F5` 运行

> 预设配置已写入 `CMakePresets.json`，包含 vcpkg toolchain、Qt 路径、运行环境 PATH，VS 会自动读取。

### 直接运行

```cmd
D:\blackholecpp\run_blackhole.bat
```

## 关键修改说明

针对 Windows 平台，对原始 Linux/WSL 代码做了以下适配：

| 修改 | 文件 | 原因 |
|------|------|------|
| `:/shaders/...` 替换 `../shaders/...` | `glcirclewidget.cpp` | 着色器从 Qt 资源加载，而非文件系统相对路径 |
| 着色器手动读取 `QFile` + `addShaderFromSourceCode` | `glcirclewidget.cpp` | 与正常工作的 widget 统一加载模式 |
| 补全 `shaders.qrc` | `shaders.qrc` | 添加 6 个缺失的着色器文件 |
| `CGAL_USE_FILE` 条件化 | `CMakeLists.txt` | 新版 CGAL (vcpkg) 不再设置该变量 |
| `dλ` → `dlambda` | `GL2DLensingWidget.h/.cpp` | MSVC 不允许 Unicode 标识符 |
| 移除 `GL`，添加 `opengl32` | `CMakeLists.txt` | Windows OpenGL 链接名不同 |
| 编译后自动复制 DLL | `CMakeLists.txt` | Qt5 + OpenMesh DLL 自动拷贝到输出目录 |
| 运行环境 PATH | `CMakePresets.json` | VS 内部运行时能找到所有 DLL |

## 项目结构

```
D:\blackholecpp\
├── CMakeLists.txt          # CMake 构建配置
├── CMakePresets.json       # VS CMake 预设（含环境变量）
├── shaders.qrc             # Qt 着色器资源文件
├── run_blackhole.bat       # 一键运行脚本
├── BUILD_WINDOWS.md        # 本文件
├── main.cpp                # 入口
├── mainwindow.h/.cpp       # 主窗口（4 个 Tab）
├── glwidget/
│   ├── glcirclewidget.*    # 黑洞渲染 (OpenGL 4.3 Core)
│   ├── GL2DLensingWidget.* # 2D 引力透镜
│   ├── glbasicwidget.*     # Basic Demo
│   └── glmultipasswidget.* # Multi-Pass Demo
├── tabs/
│   ├── controlpanel.*      # 黑洞控制面板
│   ├── LensingControlPanel.*
│   ├── basiccontrolpanel.*
│   └── multipasscontrolpanel.*
├── shaders/
│   ├── circle.frag/vert    # 黑洞着色器（主渲染）
│   ├── screen.frag/vert    # 屏幕输出
│   ├── screen_result.frag  # Bloom 合成输出
│   ├── mipmap.frag         # Mipmap 降采样
│   ├── horizontal.frag     # 水平模糊
│   ├── vertical.frag       # 垂直模糊
│   ├── basic.frag/vert     # Basic Demo
│   └── multipass*.frag/vert # Multi-Pass Demo
└── build/                  # 构建产物（gitignore）
    ├── objViewer.sln       # VS 解决方案
    └── Release/
        └── objViewer.exe   # 可执行文件
```
