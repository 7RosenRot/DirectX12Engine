$ErrorActionPreference = "Stop"
$BuildType = "RelWithDebInfo"
$ProjectName = "DIrectX12Engine"
$BuildDir = "build"

Remove-Item -Recurse -Force "build"

Write-Host ""
Write-Host "-----------------> Set CMake Properties | Configuration: $BuildType, Build directory: $BuildDir..."
Write-Host ""
cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType

Write-Host ""
Write-Host "-----------------> Assembling Project $ProjectName..."
Write-Host ""
cmake --build $BuildDir --parallel

Write-Host ""
Write-Host "-----------------> Executing..."
Write-Host ""
./$BuildDir/Debug/DirectX12Engine.exe
