# Aether Setup Guide

## Prerequisites

### Windows Development
- Windows 10/11 (x64)
- Visual Studio 2022 with C++ workload
- .NET 8.0 SDK
- Git
- CMake 3.20+

### Linux Development (Cross-compilation)
- Ubuntu 20.04+ or equivalent
- GCC 9+ or Clang 12+
- .NET 8.0 SDK
- CMake 3.20+
- vcpkg

## Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/Lain-droid/Aether.git
cd Aether
git submodule update --init --recursive
```

### 2. Install Dependencies

#### Windows
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:/vcpkg
C:/vcpkg/bootstrap-vcpkg.bat

# Install dependencies
C:/vcpkg/vcpkg install --triplet=x64-windows
```

#### Linux
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git /tmp/vcpkg
/tmp/vcpkg/bootstrap-vcpkg.sh

# Install dependencies
/tmp/vcpkg/vcpkg install --triplet=x64-linux
```

### 3. Build Project

#### Windows (Full Build)
```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release --parallel

# Run
./build/Release/Aether.exe
```

#### Linux (Cross-compilation)
```bash
# Configure for Windows target
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/tmp/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build (backend will be skipped on Linux)
cmake --build build --config Release
```

### 4. Development

#### Frontend Only
```bash
cd src/frontend
dotnet build Aether.Frontend.csproj
dotnet run
```

#### Backend Only
```bash
cd src/backend
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

## Project Structure

```
Aether/
├── src/
│   ├── backend/          # C++ Backend (Windows only)
│   ├── frontend/         # C# WPF Frontend
│   └── frontend.tests/   # Frontend tests
├── tests/               # Backend tests
├── docs/               # Documentation
├── scripts/            # Build scripts
└── .github/workflows/  # CI/CD
```

## Troubleshooting

### Common Issues

1. **vcpkg not found**
   - Ensure vcpkg is installed and in PATH
   - Use absolute path to vcpkg.cmake

2. **Dependencies not found**
   - Run `vcpkg install --triplet=x64-windows`
   - Check vcpkg.json for correct versions

3. **Frontend build fails**
   - Install .NET 8.0 SDK
   - Run `dotnet restore` first

4. **Backend build fails on Linux**
   - This is expected - backend is Windows-only
   - Use cross-compilation for Windows targets

### Debug Mode

```bash
# Debug build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Frontend debug
dotnet build --configuration Debug
```

## Contributing

1. Fork the repository
2. Create feature branch
3. Make changes
4. Run tests
5. Submit pull request

## License

MIT License - see LICENSE file for details.