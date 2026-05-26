@echo off
setlocal

set "ROOT=%~dp0.."
set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "QT5_DIR=C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\lib\cmake\Qt5"

"%CMAKE_EXE%" -S "%ROOT%" -B "%ROOT%\build" -G "Visual Studio 17 2022" -A x64 -DQt5_DIR="%QT5_DIR%"
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" --build "%ROOT%\build" --config Release --target PointCloudViewer
exit /b %errorlevel%
