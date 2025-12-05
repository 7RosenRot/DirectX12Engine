$ErrorActionPreference = "Stop"
$ProjectName = "DIrectX12Engine"
$BuildDir = "build"

Write-Host ""
Write-Host "-----------------> Assembling Project $ProjectNam..."
Write-Host ""
cmake --build $BuildDir --parallel

Write-Host ""
Write-Host "-----------------> Executing..."
Write-Host ""
./$BuildDir/Debug/DirectX12Engine.exe
