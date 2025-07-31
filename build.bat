@echo off
echo Building QuantaCerebra...

REM Try different compiler approaches
echo Attempting compilation with g++...
g++ -std=c++11 -I. main.cpp json_logic.cpp video_logic.cpp -o QuantaCerebra.exe 2>build_errors.txt

if exist QuantaCerebra.exe (
    echo Build successful! QuantaCerebra.exe created.
    echo.
    echo Running application...
    QuantaCerebra.exe
) else (
    echo Build failed. Check build_errors.txt for details.
    echo.
    echo Alternative: Try compiling with Visual Studio or another C++ compiler:
    echo cl /EHsc main.cpp json_logic.cpp video_logic.cpp /Fe:QuantaCerebra.exe
    echo.
    echo Or use online compiler like repl.it, ideone.com, or compiler-explorer.com
)

pause
