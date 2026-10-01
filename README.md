# Graphics Programming 1 - Getting Started

## Prerequisites

### Required Software
- [**Python** 3.10+](https://www.python.org/downloads/)
- [**CMake** 3.25+](https://cmake.org/download/)

> [!NOTE]
> [Conan](https://conan.io/) (C++ package manager) and [Ninja](https://ninja-build.org/) (build system) are installed automatically by `bootstrap.py` if not already present.

### Supported Compilers

Prebuilt dependencies are available for the following compiler configurations only:

#### Windows
- [**MSVC 19.5**](https://visualstudio.microsoft.com/) (via Visual Studio 2026)
- [**MinGW-w64 GCC 16**](https://www.msys2.org/) (via MSYS2)

#### Linux
- **GCC 13** (limited **GCC 15** support)
- **Clang 18**

### Verifying Your Compiler

**GCC** (Windows MSYS2 / Linux):
```powershell
gcc --version
```

**MSVC** (Windows - requires <u>PowerShell</u>):
```powershell
(& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -all -products * -prerelease -format json | ConvertFrom-Json) | ForEach-Object { $cl = (Get-ChildItem "$($_.installationPath)\VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe" -EA SilentlyContinue | Select-Object -Last 1); if($cl){ $v = & $cl.FullName 2>&1 | Select-String "Compiler Version"; "$($_.displayName) - $v" } }
```

> [!IMPORTANT]
> While other compiler versions can be built locally, this will significantly increase the build time.
> Using the version(s) defined above, you will be able to use the **private Conan server** to acquire pre-built binaries (see below).

### Recommended IDEs
- [**Visual Studio 2026**](https://visualstudio.microsoft.com/) (Windows)
- [**CLion**](https://www.jetbrains.com/clion/) (Windows / Linux)
- [**Visual Studio Code**](https://code.visualstudio.com/) with CMake Tools and C/C++ extensions (Windows / Linux)

> [!TIP]
> All listed IDEs support **CMake Presets**, which this project generates automatically.
> After running `bootstrap.py`, open the project folder and select the desired configure/build preset.

---

## Quick Start

The `bootstrap.py` script sets up everything you need. This includes a Python virtual environment, Conan, dependencies, and CMake presets.

### 1. First-time setup

**Windows (MSVC) - most common:**
```powershell
python bootstrap.py
```
This defaults to MSVC with both Debug and Release configurations.

**GCC** (Windows via MSYS2 or Linux):
```powershell
python bootstrap.py --compilers gcc
```

**Linux (Clang):**
```powershell
python3 bootstrap.py --compilers clang
```

> [!IMPORTANT]
> During first-time setup, the script will prompt you to log in to the **private Conan server**.
> Login credentials will be provided separately by your instructor.

### 2. Open in your IDE

After bootstrap completes, open the project folder in your IDE. It will detect the generated **CMake Presets** automatically, just pick your compiler and build type (Debug/Release).

### 3. Build and run

Use your IDE's build button, or from the command line:
```powershell
# configure and build (adjust preset name to match your setup)
cmake --preset conan-windows-release  
cmake --build --preset conan-windows-release

# similar on linux though there gcc is specified in the preset name
cmake --preset conan-linux-gcc
cmake --build --preset conan-linux-gcc-debug
cmake --build --preset conan-linux-gcc-release

# run the executable (adjust path to match your setup), for example on linux:
./build/gcc/bin/Release/GraphicsProgramming # Release
./build/gcc/bin/Debug/GraphicsProgramming   # Debug
```

---

## Common Options

| Flag | Description |
|------|-------------|
| `--compilers msvc gcc` | Generate for multiple compilers at once |
| `--build_types debug` | Only generate Debug (skips Release) |
| `--verbose` | Show full Conan and build output |
| `--no-conan-remote` | Skip private Conan server, use conan-center only |

## Cleaning Up

```powershell
# Clean, then re-run full setup
python bootstrap.py --clean
# Remove build folders and venv
python bootstrap.py --clean-only
# Also wipe the global Conan cache!!
python bootstrap.py --clean --clear-conan-cache
python bootstrap.py --clean-only --clear-conan-cache
```
