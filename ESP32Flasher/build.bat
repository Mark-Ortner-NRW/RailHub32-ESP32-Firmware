@echo off
echo ============================================
echo    RailHub32 Flasher - Build Script
echo ============================================
echo.

cd /d "%~dp0"

echo [1/3] Restoring dependencies...
dotnet restore
if errorlevel 1 (
    echo Error: Failed to restore dependencies
    pause
    exit /b 1
)

echo.
echo [2/3] Building application...
dotnet build -c Release
if errorlevel 1 (
    echo Error: Build failed
    pause
    exit /b 1
)

echo.
echo [3/3] Publishing standalone executable...
dotnet publish -c Release -r win-x64 --self-contained -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true
if errorlevel 1 (
    echo Error: Publish failed
    pause
    exit /b 1
)

echo.
echo ============================================
echo   Build Complete!
echo ============================================
echo.
echo Executable location:
echo bin\Release\net8.0-windows\win-x64\publish\RailHub32 Flasher.exe
echo.
echo You can distribute this single .exe file!
echo.
pause
