CONFIGURE=0
RELEASE=0
SHADERS=0

function usage {
    echo "Usage: $0 [-c] [-r]"
    echo "  -c: Whether to configure the project or just build it."
    echo "  -r: Whether to build a release or debug version of the project."
    echo "  -s: Whether or not to compile the shaders for the project."
    exit $1
}


while getopts crsh option
do
    case $option in
    c) CONFIGURE=1;;
    r) RELEASE=1;;
    s) SHADERS=1;;
    h) usage 0;;
    *) usage 1;;
    esac
done

if [ $CONFIGURE -eq 1 ]; then
    BUILD_TYPE="Debug"
    if [ $RELEASE -eq 1 ]; then
        BUILD_TYPE="Release"
    fi

    echo "Configuring project..."
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -S . -B build
fi

echo "Building project..."
cmake --build build -- -j6