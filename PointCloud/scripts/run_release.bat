@echo off
setlocal

set "ROOT=%~dp0.."
set "QT_BIN=C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin"
set "PCL_BIN=D:\library\PCL 1.10.0\bin"
set "VTK_BIN=D:\library\PCL 1.10.0\3rdParty\VTK\bin"
set "OPENNI_BIN=D:\library\PCL 1.10.0\3rdParty\OpenNI2\Redist"

set "PATH=%QT_BIN%;%PCL_BIN%;%VTK_BIN%;%OPENNI_BIN%;%PATH%"
cd /d "%ROOT%"

if not exist "%ROOT%\build\Release\PointCloudViewer.exe" (
    echo Release executable was not found. Run scripts\build_release.bat first.
    exit /b 1
)

"%ROOT%\build\Release\PointCloudViewer.exe"
