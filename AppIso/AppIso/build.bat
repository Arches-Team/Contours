@echo off
setlocal enabledelayedexpansion

rem One-shot build script: locates Qt and Visual Studio, sets up the MSVC
rem environment, and builds AppIso with CMake. Just run this file -- no
rem need to open a special developer prompt.
rem
rem Qt is NOT installed automatically. Install it yourself first (see
rem README.md), then either let this script find it under C:\Qt\, or set
rem the QT_DIR environment variable to your Qt kit folder, e.g.:
rem   set QT_DIR=C:\Qt\6.8.3\msvc2022_64

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"

echo === AppIso build ===
echo.

rem --- Step 1: find Qt ----------------------------------------------------
set "QT_PREFIX="
if defined QT_DIR (
    if exist "%QT_DIR%\bin\qmake.exe" set "QT_PREFIX=%QT_DIR%"
)

if not defined QT_PREFIX (
    if exist "C:\Qt" (
        for /d %%V in ("C:\Qt\*") do (
            if exist "%%V\msvc2022_64\bin\qmake.exe" set "QT_PREFIX=%%V\msvc2022_64"
        )
    )
)

if defined QT_PREFIX (
    echo Found Qt at "!QT_PREFIX!".
) else (
    echo ERROR: Qt 6 was not found.
    echo.
    echo Install Qt 6 for MSVC 2022 64-bit ^(modules: Core, Gui, Widgets, OpenGL,
    echo OpenGLWidgets, Svg^) from https://www.qt.io/download-qt-installer
    echo.
    echo Then either install it under C:\Qt\ ^(the default location^), or set the
    echo QT_DIR environment variable to your Qt kit folder, for example:
    echo   set QT_DIR=C:\Qt\6.8.3\msvc2022_64
    echo and re-run this script.
    goto :fail
)
echo.

rem --- Step 2: find Visual Studio 2022 and set up the MSVC environment --
echo Looking for Visual Studio 2022...
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Visual Studio Installer was not found.
    echo Install Visual Studio 2022 ^(Community edition is free^) with the
    echo "Desktop development with C++" workload: https://visualstudio.microsoft.com/downloads/
    goto :fail
)

set "VS_PATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)
if not defined VS_PATH (
    echo ERROR: No Visual Studio installation with the "Desktop development with C++"
    echo workload was found. Install it via the Visual Studio Installer.
    goto :fail
)

set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    echo ERROR: vcvars64.bat not found under "%VS_PATH%".
    goto :fail
)

echo Setting up the MSVC build environment...
call "%VCVARS%" >nul
if errorlevel 1 (
    echo ERROR: failed to initialize the MSVC environment.
    goto :fail
)
echo.

rem --- Step 3: configure and build ---------------------------------------

rem If a previous build/ directory was configured with a different compiler
rem (e.g. from a terminal without the MSVC environment set up), CMake would
rem silently keep reusing it instead of picking up cl.exe. Detect that and
rem reset the directory so the build always uses MSVC.
if exist "%BUILD_DIR%\CMakeCache.txt" (
    findstr /I /C:"CMAKE_CXX_COMPILER:FILEPATH=" "%BUILD_DIR%\CMakeCache.txt" | findstr /I /C:"cl.exe" >nul
    if errorlevel 1 (
        echo Existing "%BUILD_DIR%" was configured with a different compiler, resetting it...
        rmdir /s /q "%BUILD_DIR%"
    )
)

where ninja >nul 2>nul
if errorlevel 1 (set "GENERATOR=NMake Makefiles") else (set "GENERATOR=Ninja")
echo Using CMake generator: %GENERATOR%

echo Configuring...
cmake -S "%SCRIPT_DIR%." -B "%BUILD_DIR%" -G "%GENERATOR%" -DCMAKE_PREFIX_PATH="%QT_PREFIX%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    goto :fail
)

echo.
echo Building...
cmake --build "%BUILD_DIR%" --parallel
if errorlevel 1 (
    echo ERROR: build failed.
    goto :fail
)

echo.
echo ================================================
echo  Build succeeded!
echo  Executable: %BUILD_DIR%\AppIso.exe
echo ================================================
pause
exit /b 0

:fail
echo.
pause
exit /b 1
