@echo off
setlocal

echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if %ERRORLEVEL% NEQ 0 (
    echo Failed to setup VS environment
    exit /b 1
)

echo.
echo Building with CMake...
cd /d "%~dp0build"
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build . --config Release -- -j4
if %ERRORLEVEL% NEQ 0 (
    echo Build failed
    exit /b 1
)

echo.
echo Build completed successfully!
