@echo off

set CONFIGURE=0
set RELEASE=0
set INSTALL=0

:getopts
if /I "%1" == "-c" set CONFIGURE=1
if /I "%1" == "-r" set RELEASE=1
if /I "%1" == "-i" set INSTALL=1
if /I "%1" == "-h" (call :usage 0)
else (call :usage 1)
shift
if not "%1" == "" goto :getopts


if %CONFIGURE% == 1 (
    set BUILD_TYPE=Debug
    if %RELEASE% == 1 set BUILD_TYPE=Release

    set OPTIONS="-DCMAKE_BUILD_TYPE=%BUILD_TYPE%"
    if %INSTALL% == 1 set OPTIONS=%OPTIONS% "-DCPAK_INSTALL=ON"

    echo "Configuring project..."
    cmake -S . -B build -G "Visual Studio 17 2022" %OPTIONS%
)

echo "Building project..."
cmake --build ./build -- -j6

if %INSTALL% == 1 (
    echo "Installing project..."
    cmake --install ./build
)

exit /B 0

:usage
echo "Usage: %~nx0 [-c] [-r] [-i] [-h]"
echo "  -c: Whether to configure the project or just build it."
echo "  -r: Whether to build a release or debug version of the project."
echo "  -s: Whether or not to compile the shaders for the project."
exit /B %~1
