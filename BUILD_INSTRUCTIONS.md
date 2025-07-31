# QuantaCerebra Build Instructions

## Overview

QuantaCerebra is a C++11/17 console application for visualizing brain activity through ASCII animations. This document provides comprehensive build instructions and troubleshooting guidance.

## Build Requirements

### Minimum Requirements
- C++11 compatible compiler
- Standard C++ library
- No external dependencies required

### Recommended Compilers
- **GCC 7+** (Linux/Windows)
- **Clang 5+** (macOS/Linux)
- **MSVC 2017+** (Windows)
- **Online Compilers** (as fallback)

## Build Methods

### Method 1: Direct Compilation (Recommended)

#### Single File Version (Easiest)
```bash
g++ -std=c++11 simple_build.cpp -o QuantaCerebra
```

#### Modular Version
```bash
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra
```

### Method 2: Using Makefile
```bash
make                    # Build main application
make test              # Build and run tests
make run               # Build and run application
make clean             # Clean build artifacts
make windows           # Windows-specific build
```

### Method 3: Using Batch Script (Windows)
```cmd
build.bat
```

### Method 4: Manual Compilation Steps
```bash
# Compile object files
g++ -std=c++11 -c main.cpp -o main.o
g++ -std=c++11 -c json_logic.cpp -o json_logic.o
g++ -std=c++11 -c video_logic.cpp -o video_logic.o

# Link executable
g++ main.o json_logic.o video_logic.o -o QuantaCerebra
```

## Platform-Specific Instructions

### Windows

#### Option 1: Visual Studio
```cmd
cl /EHsc main.cpp json_logic.cpp video_logic.cpp /Fe:QuantaCerebra.exe
```

#### Option 2: MinGW (if working)
```cmd
g++ -std=c++11 -static-libgcc -static-libstdc++ main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra.exe
```

#### Option 3: MSYS2
```bash
pacman -S mingw-w64-x86_64-gcc
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra.exe
```

### Linux
```bash
# Ubuntu/Debian
sudo apt install build-essential
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra

# CentOS/RHEL
sudo yum install gcc-c++
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra
```

### macOS
```bash
# Using Xcode Command Line Tools
xcode-select --install
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra

# Using Homebrew
brew install gcc
g++ -std=c++11 main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra
```

## Online Compilation (Fallback)

If local compilation fails, use these online compilers:

### Recommended Online Compilers
1. **Compiler Explorer** (https://godbolt.org/)
   - Paste `simple_build.cpp` content
   - Select GCC or Clang compiler
   - Add execution arguments if needed

2. **Repl.it** (https://replit.com/)
   - Create new C++ project
   - Upload source files
   - Click Run

3. **OnlineGDB** (https://www.onlinegdb.com/)
   - Select C++ language
   - Paste code and run

4. **Ideone** (https://ideone.com/)
   - Select C++17 or C++14
   - Paste code and execute

## Troubleshooting

### Common Issues

#### Issue 1: MinGW Compilation Errors
**Symptoms:** `__MINGW_EXTENSION` errors, `size_t` undefined
**Solution:** 
- Use single-file version: `g++ -std=c++11 simple_build.cpp -o QuantaCerebra`
- Or switch to Visual Studio/MSVC
- Or use online compiler

#### Issue 2: C++11 Features Not Recognized
**Symptoms:** `auto` keyword errors, range-based for loops fail
**Solution:**
- Ensure `-std=c++11` flag is used
- Update compiler to GCC 4.8+ or equivalent

#### Issue 3: File Not Found Errors
**Symptoms:** Cannot open `sample_input.json`
**Solution:**
- Ensure `sample_input.json` is in the same directory as executable
- Check file permissions
- Verify file contents are valid JSON

#### Issue 4: Linking Errors
**Symptoms:** Undefined reference errors
**Solution:**
- Compile all source files together
- Check that all `.cpp` files are included in compilation command

### Build Verification

After successful compilation, verify the build:

```bash
# Check executable exists
ls -la QuantaCerebra*

# Test run (should show brain activity visualization)
./QuantaCerebra

# Run tests (if compiled)
./test_unit
./test_bdd
```

## File Structure

```
QuantaCerebra/
├── main.cpp                    # Main application
├── json_logic.h/.cpp          # JSON parsing
├── video_logic.h/.cpp         # Visualization
├── simple_build.cpp           # Single-file version
├── test_unit.cpp              # Unit tests
├── test_bdd.cpp               # BDD tests
├── sample_input.json          # Test data
├── expected_output.txt        # Expected output
├── Makefile                   # Build automation
├── build.bat                  # Windows build script
└── BUILD_INSTRUCTIONS.md      # This file
```

## Expected Output

When running successfully, you should see:

```
QuantaCerebra - Brain Activity Visualizer
==========================================

Visualizing 2 brain activity frames:

[Time: 0 ms]
[prefrontal_cortex] XXXXXXXX
[amygdala] ++++++
[occipital_lobe] @@@@@@@@@

[Time: 66 ms]
[prefrontal_cortex] +++++
[amygdala] ...
[occipital_lobe] ..

Visualization complete!
```

## Performance Notes

- The application uses a simple busy-wait loop for timing
- For production use, consider implementing proper timing mechanisms
- Memory usage is minimal (< 1MB typical)
- CPU usage is low except during delay loops

## Development Notes

- Code follows C++11 standards for maximum compatibility
- No external dependencies to ensure portability
- Manual JSON parsing avoids library dependencies
- ASCII visualization keeps output simple and universal

## Support

If you encounter build issues:

1. Try the single-file version first (`simple_build.cpp`)
2. Use online compilers as a fallback
3. Check compiler version and C++11 support
4. Verify all source files are present
5. Ensure `sample_input.json` exists and is valid

For additional help, refer to your compiler's documentation or use online development environments.
