# Copy Qt DLLs from Qt installation (Qt 6.10.0)
$qtBinPath = "C:\Qt\6.10.0\msvc2022_64\bin"
$qtPluginsPath = "C:\Qt\6.10.0\msvc2022_64\plugins"
$dest = "C:\dev\osandell\albert\build\bin"

if (Test-Path $qtBinPath) {
    Write-Host "Copying Qt DLLs from Qt 6.10.0..."
    Copy-Item "$qtBinPath\Qt6*.dll" $dest -Force
    Write-Host "Copying Qt6Keychain DLLs..."
    Copy-Item "$qtBinPath\qt6keychain*.dll" $dest -Force -ErrorAction SilentlyContinue
    Write-Host "Qt DLLs copied successfully"
}

# Copy Qt platform plugins (required for Windows platform plugin)
if (Test-Path $qtPluginsPath) {
    $platformsDest = Join-Path $dest "platforms"
    if (-not (Test-Path $platformsDest)) {
        New-Item -ItemType Directory -Path $platformsDest -Force | Out-Null
    }
    Write-Host "Copying Qt platform plugins..."
    Copy-Item "$qtPluginsPath\platforms\qwindows*.dll" $platformsDest -Force
    Write-Host "Qt platform plugins copied successfully"
    
    # Copy Qt SQL plugins (required for SQLite database driver)
    $sqldriversDest = Join-Path $dest "sqldrivers"
    if (-not (Test-Path $sqldriversDest)) {
        New-Item -ItemType Directory -Path $sqldriversDest -Force | Out-Null
    }
    Write-Host "Copying Qt SQL plugins..."
    Copy-Item "$qtPluginsPath\sqldrivers\qsqlite*.dll" $sqldriversDest -Force
    Write-Host "Qt SQL plugins copied successfully"
    
    # Copy Qt imageformats plugins (required for SVG icon support)
    $imageformatsDest = Join-Path $dest "imageformats"
    if (-not (Test-Path $imageformatsDest)) {
        New-Item -ItemType Directory -Path $imageformatsDest -Force | Out-Null
    }
    Write-Host "Copying Qt imageformats plugins..."
    Copy-Item "$qtPluginsPath\imageformats\qsvg*.dll" $imageformatsDest -Force
    Write-Host "Qt imageformats plugins copied successfully"
}

# Copy vcpkg DLLs (but exclude Qt DLLs to avoid version conflicts)
$vcpkgBinPath = "C:\vcpkg\installed\x64-windows\bin"
if (Test-Path $vcpkgBinPath) {
    Write-Host "Copying vcpkg DLLs (excluding Qt DLLs)..."
    Get-ChildItem "$vcpkgBinPath\*.dll" | Where-Object { $_.Name -notlike "Qt6*" } | Copy-Item -Destination $dest -Force -ErrorAction SilentlyContinue
    # Explicitly copy qt6keychain DLLs if present
    Copy-Item "$vcpkgBinPath\qt6keychain*.dll" $dest -Force -ErrorAction SilentlyContinue
    Write-Host "vcpkg DLLs copied successfully"
}

# Also check debug bin directory for debug DLLs (excluding Qt DLLs)
$vcpkgDebugBinPath = "C:\vcpkg\installed\x64-windows\debug\bin"
if (Test-Path $vcpkgDebugBinPath) {
    Write-Host "Copying vcpkg debug DLLs (excluding Qt DLLs)..."
    Get-ChildItem "$vcpkgDebugBinPath\*.dll" | Where-Object { $_.Name -notlike "Qt6*" } | Copy-Item -Destination $dest -Force -ErrorAction SilentlyContinue
    # Explicitly copy qt6keychain debug DLLs if present
    Copy-Item "$vcpkgDebugBinPath\qt6keychain*.dll" $dest -Force -ErrorAction SilentlyContinue
    Write-Host "vcpkg debug DLLs copied successfully"
}

# Copy Visual C++ runtime DLLs
$source = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\debug_nonredist\x64\Microsoft.VC143.DebugCRT"

if (Test-Path $source) {
    Write-Host "Copying Visual C++ runtime DLLs..."
    Copy-Item "$source\vcruntime140d.dll" $dest -Force -ErrorAction SilentlyContinue
    Copy-Item "$source\vcruntime140_1d.dll" $dest -Force -ErrorAction SilentlyContinue
    Copy-Item "$source\msvcp140d.dll" $dest -Force -ErrorAction SilentlyContinue
    Copy-Item "$source\ucrtbased.dll" $dest -Force -ErrorAction SilentlyContinue
    Copy-Item "$source\concrt140d.dll" $dest -Force -ErrorAction SilentlyContinue
    Write-Host "Runtime DLLs copied successfully"
}
