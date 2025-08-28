#!/usr/bin/env pwsh
# Aether Advanced Build Script
# Supports Debug/Release builds, packaging, and distribution

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    
    [Parameter(Mandatory=$false)]
    [switch]$Clean,
    
    [Parameter(Mandatory=$false)]
    [switch]$Package,
    
    [Parameter(Mandatory=$false)]
    [switch]$Deploy,
    
    [Parameter(Mandatory=$false)]
    [switch]$RunTests,
    
    [Parameter(Mandatory=$false)]
    [string]$OutputPath = "./dist"
)

# Script configuration
$ErrorActionPreference = "Stop"
$BuildPath = "./build"
$SrcPath = "./src"
$BackendPath = "$SrcPath/backend"
$FrontendPath = "$SrcPath/frontend"

# Colors for output
function Write-ColoredOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Write-Step {
    param([string]$Step)
    Write-ColoredOutput "==> $Step" "Cyan"
}

function Write-Success {
    param([string]$Message)
    Write-ColoredOutput "✓ $Message" "Green"
}

function Write-Error {
    param([string]$Message)
    Write-ColoredOutput "✗ $Message" "Red"
}

# Build steps
function Initialize-Build {
    Write-Step "Initializing build environment"
    
    if ($Clean -and (Test-Path $BuildPath)) {
        Remove-Item $BuildPath -Recurse -Force
        Write-Success "Cleaned build directory"
    }
    
    if (-not (Test-Path $BuildPath)) {
        New-Item -Path $BuildPath -ItemType Directory -Force | Out-Null
    }
    
    if (-not (Test-Path $OutputPath)) {
        New-Item -Path $OutputPath -ItemType Directory -Force | Out-Null
    }
    
    Write-Success "Build environment initialized"
}

function Build-Backend {
    Write-Step "Building C++ Backend"
    
    Push-Location $BuildPath
    try {
        # Configure with CMake
        cmake .. -DCMAKE_BUILD_TYPE=$Configuration -DCMAKE_INSTALL_PREFIX="$OutputPath/backend"
        if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
        
        # Build
        cmake --build . --config $Configuration --parallel
        if ($LASTEXITCODE -ne 0) { throw "Backend build failed" }
        
        # Install
        cmake --install . --config $Configuration
        if ($LASTEXITCODE -ne 0) { throw "Backend install failed" }
        
        Write-Success "Backend built successfully"
    }
    finally {
        Pop-Location
    }
}

function Build-Frontend {
    Write-Step "Building C# Frontend"
    
    Push-Location $FrontendPath
    try {
        # Restore packages
        dotnet restore
        if ($LASTEXITCODE -ne 0) { throw "NuGet restore failed" }
        
        # Build
        $buildConfig = if ($Configuration -eq "Debug") { "Debug" } else { "Release" }
        dotnet build --no-restore --configuration $buildConfig --output "$OutputPath/frontend"
        if ($LASTEXITCODE -ne 0) { throw "Frontend build failed" }
        
        Write-Success "Frontend built successfully"
    }
    finally {
        Pop-Location
    }
}

function Run-Tests {
    if (-not $RunTests) { return }
    
    Write-Step "Running tests"
    
    # Backend tests (if exists)
    if (Test-Path "$BuildPath/tests") {
        Push-Location $BuildPath
        try {
            ctest --output-on-failure --parallel
            if ($LASTEXITCODE -ne 0) { throw "Backend tests failed" }
        }
        finally {
            Pop-Location
        }
    }
    
    # Frontend tests
    if (Test-Path "$FrontendPath.Tests") {
        Push-Location "$FrontendPath.Tests"
        try {
            dotnet test --no-build --configuration $Configuration
            if ($LASTEXITCODE -ne 0) { throw "Frontend tests failed" }
        }
        finally {
            Pop-Location
        }
    }
    
    Write-Success "All tests passed"
}

function Create-Package {
    if (-not $Package) { return }
    
    Write-Step "Creating distribution package"
    
    $PackagePath = "$OutputPath/Aether-v1.0.0-$Configuration"
    
    if (Test-Path $PackagePath) {
        Remove-Item $PackagePath -Recurse -Force
    }
    
    New-Item -Path $PackagePath -ItemType Directory -Force | Out-Null
    
    # Copy backend
    if (Test-Path "$OutputPath/backend") {
        Copy-Item "$OutputPath/backend/*" $PackagePath -Recurse
    }
    
    # Copy frontend
    if (Test-Path "$OutputPath/frontend") {
        Copy-Item "$OutputPath/frontend/*" $PackagePath -Recurse
    }
    
    # Copy documentation
    Copy-Item "./docs" "$PackagePath/docs" -Recurse -ErrorAction SilentlyContinue
    Copy-Item "./README.md" $PackagePath -ErrorAction SilentlyContinue
    
    # Create installer script
    @"
@echo off
echo Aether Installation
echo.
echo Installing to %PROGRAMFILES%\Aether...
mkdir "%PROGRAMFILES%\Aether" 2>nul
xcopy /E /Y *.* "%PROGRAMFILES%\Aether\"
echo.
echo Installation complete!
echo Run: "%PROGRAMFILES%\Aether\Aether.exe"
pause
"@ | Out-File "$PackagePath/install.bat" -Encoding ASCII
    
    # Create ZIP package
    if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
        $ZipPath = "$OutputPath/Aether-v1.0.0-$Configuration.zip"
        Compress-Archive -Path "$PackagePath/*" -DestinationPath $ZipPath -Force
        Write-Success "Package created: $ZipPath"
    }
    
    Write-Success "Distribution package created"
}

function Deploy-Application {
    if (-not $Deploy) { return }
    
    Write-Step "Deploying application"
    
    # This could upload to a server, update registry, etc.
    # For now, just copy to a standard location
    $DeployPath = "$env:PROGRAMFILES\Aether"
    
    if (Test-Path $DeployPath) {
        Remove-Item $DeployPath -Recurse -Force
    }
    
    Copy-Item "$OutputPath/frontend" $DeployPath -Recurse
    Copy-Item "$OutputPath/backend" "$DeployPath/backend" -Recurse -ErrorAction SilentlyContinue
    
    Write-Success "Application deployed to $DeployPath"
}

# Main execution
try {
    Write-ColoredOutput "🚀 Aether Build System v1.0" "Magenta"
    Write-ColoredOutput "Configuration: $Configuration" "Yellow"
    Write-ColoredOutput "Output: $OutputPath" "Yellow"
    Write-Host ""
    
    Initialize-Build
    Build-Backend
    Build-Frontend
    Run-Tests
    Create-Package
    Deploy-Application
    
    Write-Host ""
    Write-ColoredOutput "🎉 Build completed successfully!" "Green"
    
    if ($Package) {
        Write-ColoredOutput "📦 Package available in: $OutputPath" "Yellow"
    }
}
catch {
    Write-Host ""
    Write-Error "Build failed: $($_.Exception.Message)"
    exit 1
}