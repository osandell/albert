@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 > build_log.txt 2>&1
cd /d "C:\dev\osandell\albert" >> build_log.txt 2>&1
echo Configuring with CMake... >> build_log.txt 2>&1
cmake -S . -B build -G Ninja >> build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed! >> build_log.txt 2>&1
    exit /b 1
)
echo Building... >> build_log.txt 2>&1
cmake --build build >> build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Build failed! >> build_log.txt 2>&1
    exit /b 1
)
echo Build completed successfully! >> build_log.txt 2>&1
type build_log.txt
