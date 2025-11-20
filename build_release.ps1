# Build script for Albert (Release mode)
Remove-Item -Recurse -Force 'C:\dev\osandell\albert\build' -ErrorAction SilentlyContinue

& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -SkipAutomaticLocation

Set-Location 'C:\dev\osandell\albert'

& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.10.0\msvc2022_64;C:\vcpkg\installed\x64-windows" `
  -DBUILD_PLUGIN_CALCULATOR-QALCULATE=OFF `
  -DBUILD_PLUGIN_WIDGETSBOXMODEL-QSS=OFF `
  -DBUILD_PLUGIN_MEDIAREMOTE=OFF `
  -DBUILD_PLUGIN_OBSIDIAN=OFF `
  -DBUILD_PLUGIN_APPLICATION=OFF `
  -DBUILD_PLUGIN_APPLICATIONS=OFF `
  -DBUILD_PLUGIN_PYTHON=OFF `
  -DBUILD_PLUGIN_FILES=OFF `
  -DBUILD_PLUGIN_PATH=OFF `
  -DBUILD_PLUGIN_SSH=OFF

if ($LASTEXITCODE -eq 0) {
    & 'C:\portableapps\ninja\ninja.exe' -C build
}
