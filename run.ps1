$ErrorActionPreference = "Stop"

Remove-Item -Recurse -Force "build"

Write-Host ""
Write-Host "-----------------> Set CMake Properties..."
Write-Host ""
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

Write-Host ""
Write-Host "-----------------> Assembling Project..."
Write-Host ""
cmake --build build --parallel

Write-Host ""
Write-Host "-----------------> Executing..."
Write-Host ""
./build/Debug/DirectX12Engine.exe
