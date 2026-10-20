@echo off
cd /d "%~dp0"

echo Compiling SmartCityMST...

g++ -std=c++17 server.cpp -o server.exe -lws2_32

if errorlevel 1 (
    echo.
    echo Compilation failed. Check the errors above.
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo Starting SmartCityMST server...
echo.

server.exe

pause