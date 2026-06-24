@echo off
setlocal

set BUILD_DIR=%1
if "%BUILD_DIR%"=="" set BUILD_DIR=build

set CONFIG=%2
if "%CONFIG%"=="" set CONFIG=Release

echo.
echo =^> Initializing submodules...
git submodule update --init --recursive
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
echo =^> Configuring CMake (no build)...
cmake -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%CONFIG%
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
echo Done! Project configured in '%BUILD_DIR%'.
echo To build: cmake --build %BUILD_DIR% --config %CONFIG% --parallel

endlocal
