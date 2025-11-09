$source = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\debug_nonredist\x64\Microsoft.VC143.DebugCRT"
$dest = "C:\dev\osandell\albert\build\bin"

Copy-Item "$source\vcruntime140d.dll" $dest -Force
Copy-Item "$source\vcruntime140_1d.dll" $dest -Force
Copy-Item "$source\msvcp140d.dll" $dest -Force
Copy-Item "$source\ucrtbased.dll" $dest -Force
Copy-Item "$source\concrt140d.dll" $dest -Force -ErrorAction SilentlyContinue

Write-Host "Runtime DLLs copied successfully"
