@echo off
REM Black Hole Viewer - Run Script
REM Sets up PATH for Qt5, vcpkg, and launches the application

set "QT_DIR=D:\Qt\5.11.1\msvc2017_64"
set "VCPKG_DIR=C:\Users\Administrator\vcpkg\installed\x64-windows"

REM Add Qt and vcpkg to PATH
set "PATH=%QT_DIR%\bin;%VCPKG_DIR%\bin;%PATH%"

REM Set Qt plugin path
set "QT_PLUGIN_PATH=%QT_DIR%\plugins"

REM Launch the application
start "" "D:\blackholecpp\build\Release\objViewer.exe"

echo Black Hole Viewer launched!
echo If you see DLL errors, check that Qt5 and vcpkg paths are correct.
pause
