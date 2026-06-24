param(
    [string]$BuildDir = "build",
    [string]$Config   = "Release"
)

$ErrorActionPreference = "Stop"

function Step([string]$msg) {
    Write-Host "`n==> $msg" -ForegroundColor Cyan
}

Step "Initializing submodules..."
git submodule update --init --recursive
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Step "Configuring CMake (no build)..."
cmake -B $BuildDir -DCMAKE_BUILD_TYPE=$Config
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`nDone! Project configured in '$BuildDir'." -ForegroundColor Green
Write-Host "To build: cmake --build $BuildDir --config $Config --parallel" -ForegroundColor DarkGray
