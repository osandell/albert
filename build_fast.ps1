# Fast incremental build script for Albert (doesn't delete build directory)

& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -SkipAutomaticLocation

Set-Location 'C:\dev\osandell\albert'

& 'C:\portableapps\ninja\ninja.exe' -C build


if ($LASTEXITCODE -eq 0) {
    # Copy Qt DLLs from Qt installation (Qt 6.10.0)
    $qtBinPath = "C:\Qt\6.10.0\msvc2022_64\bin"
    $qtPluginsPath = "C:\Qt\6.10.0\msvc2022_64\plugins"
    $buildBinPath = "C:\dev\osandell\albert\build\bin"

    if (Test-Path $qtBinPath) {
        Write-Host "Copying Qt DLLs from Qt 6.10.0..."
        Copy-Item "$qtBinPath\Qt6*.dll" $buildBinPath -Force
        Write-Host "Copying Qt6Keychain DLLs..."
        Copy-Item "$qtBinPath\qt6keychain*.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Write-Host "Qt DLLs copied successfully"
    }

    # Copy Qt platform plugins (required for Windows platform plugin)
    if (Test-Path $qtPluginsPath) {
        $platformsDest = Join-Path $buildBinPath "platforms"
        if (-not (Test-Path $platformsDest)) {
            New-Item -ItemType Directory -Path $platformsDest -Force | Out-Null
        }
        Write-Host "Copying Qt platform plugins..."
        Copy-Item "$qtPluginsPath\platforms\qwindows*.dll" $platformsDest -Force
        Write-Host "Qt platform plugins copied successfully"

        # Copy Qt SQL plugins (required for SQLite database driver)
        $sqldriversDest = Join-Path $buildBinPath "sqldrivers"
        if (-not (Test-Path $sqldriversDest)) {
            New-Item -ItemType Directory -Path $sqldriversDest -Force | Out-Null
        }
        Write-Host "Copying Qt SQL plugins..."
        Copy-Item "$qtPluginsPath\sqldrivers\qsqlite*.dll" $sqldriversDest -Force
        Write-Host "Qt SQL plugins copied successfully"

        # Copy Qt imageformats plugins (required for SVG icon support)
        $imageformatsDest = Join-Path $buildBinPath "imageformats"
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
        Get-ChildItem "$vcpkgBinPath\*.dll" | Where-Object { $_.Name -notlike "Qt6*" } | Copy-Item -Destination $buildBinPath -Force -ErrorAction SilentlyContinue
        # Explicitly copy qt6keychain DLLs if present
        Copy-Item "$vcpkgBinPath\qt6keychain*.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Write-Host "vcpkg DLLs copied successfully"
    }

    # Also check debug bin directory for debug DLLs (excluding Qt DLLs)
    $vcpkgDebugBinPath = "C:\vcpkg\installed\x64-windows\debug\bin"
    if (Test-Path $vcpkgDebugBinPath) {
        Write-Host "Copying vcpkg debug DLLs (excluding Qt DLLs)..."
        Get-ChildItem "$vcpkgDebugBinPath\*.dll" | Where-Object { $_.Name -notlike "Qt6*" } | Copy-Item -Destination $buildBinPath -Force -ErrorAction SilentlyContinue
        # Explicitly copy qt6keychain debug DLLs if present
        Copy-Item "$vcpkgDebugBinPath\qt6keychain*.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Write-Host "vcpkg debug DLLs copied successfully"
    }

    # Copy Visual C++ runtime DLLs
    $vcRedistPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\debug_nonredist\x64\Microsoft.VC143.DebugCRT"
    if (Test-Path $vcRedistPath) {
        Write-Host "Copying Visual C++ runtime DLLs..."
        Copy-Item "$vcRedistPath\vcruntime140d.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Copy-Item "$vcRedistPath\vcruntime140_1d.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Copy-Item "$vcRedistPath\msvcp140d.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Copy-Item "$vcRedistPath\ucrtbased.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Copy-Item "$vcRedistPath\concrt140d.dll" $buildBinPath -Force -ErrorAction SilentlyContinue
        Write-Host "Runtime DLLs copied successfully"
    }

    Write-Host ""
    Write-Host "Build completed successfully!" -ForegroundColor Green
}
