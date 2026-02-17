@echo off
setlocal enabledelayedexpansion

:: Get the directory where the script is located (project root/bin/ or project root/)
set "SOURCE_ROOT=%~dp0.."
set "INSTALL_DIR=C:\ry"
set "STD_LIB_DIR=%INSTALL_DIR%\modules"

echo --- Ry Windows System-Wide Installer ---

:: Admin Check
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Please run as Administrator to install to C:\ and update PATH.
    pause
    exit /b 1
)

:: Create folders
echo Creating directories...
if not exist "%INSTALL_DIR%\bin" mkdir "%INSTALL_DIR%\bin"
if not exist "%INSTALL_DIR%\lib" mkdir "%INSTALL_DIR%\lib"
if not exist "%STD_LIB_DIR%" mkdir "%STD_LIB_DIR%"

:: Copy Binaries and Shared Libs (.exe and .dll)
echo Copying Ry binaries and core DLLs...
copy /Y "%SOURCE_ROOT%\bin\ry.exe" "%INSTALL_DIR%\bin\" >nul
if exist "%SOURCE_ROOT%\lib\*.dll" (
    copy /Y "%SOURCE_ROOT%\lib\*.dll" "%INSTALL_DIR%\lib\" >nul
)

:: Copy Standard Library (The .ry files and C++ modules)
echo Installing Ry Standard Library...
:: Copy .ry files from modules/library
if exist "%SOURCE_ROOT%\modules\library\*" (
    xcopy /Y /E "%SOURCE_ROOT%\modules\library\*" "%STD_LIB_DIR%\" >nul
)
:: Copy .dll modules from modules/lib_cpp
if exist "%SOURCE_ROOT%\modules\lib_cpp\*.dll" (
    copy /Y "%SOURCE_ROOT%\modules\lib_cpp\*.dll" "%STD_LIB_DIR%\" >nul
)

:: Set Environment Variables
echo Configuring Environment...

:: Add bin to PATH if not already there
echo %PATH% | findstr /I /C:";%INSTALL_DIR%\bin" >nul
if %errorlevel% neq 0 (
    setx /M PATH "%PATH%;%INSTALL_DIR%\bin"
)

:: Set RY_LIB_PATH so the interpreter knows where to look for 'use()'
setx /M RY_LIB_PATH "%STD_LIB_DIR%"

echo.
echo Ry installation complete! 
echo Standard Library installed to: %STD_LIB_DIR%
pause