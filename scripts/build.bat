@echo off
setlocal enabledelayedexpansion

:: --- Colors (Supported in Windows 10/11) ---
set "ESC="
set "RED=%ESC%[31m"
set "GREEN=%ESC%[32m"
set "YELLOW=%ESC%[33m"
set "BLUE=%ESC%[34m"
set "CYAN=%ESC%[36m"
set "BOLD=%ESC%[1m"
set "RESET=%ESC%[0m"

echo %BLUE%%BOLD%--- Ry Build System (Windows) ---%RESET%

set JOBS=2

mkdir build

:: Execute Build
cmake --build build -j %JOBS%

:: Success Check (%ERRORLEVEL% is the batch version of $?)
if %ERRORLEVEL% EQU 0 (
    if not exist bin mkdir bin
    if not exist lib mkdir lib
    
    copy /Y build\Debug\ry.exe bin\ >nul 2>&1
    copy /Y build\ry.exe bin\ >nul 2>&1
    copy /Y build\*.dll lib\ >nul 2>&1
    
    echo.
    echo %GREEN%%BOLD% Build complete.%RESET%
) else (
    echo.
    echo %RED%%BOLD% Build failed.%RESET% Check the errors above.
    exit /b 1
)