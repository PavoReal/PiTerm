#! /bin/bash

CC=gcc
CXX=g++
LD=g++

CXX_FLAGS="-Wall -Wextra -Wno-int-to-pointer-cast -Wno-write-strings -Wno-unknown-pragmas -I/usr/local/include/SDL2 -D_REENTRANT -DIMGUI_IMPL_OPENGL_LOADER_GL3W -DPI_TERM -DPI_TERM_INTERNAL"
CXX_FLAGS_DEBUG="-g -O0 -DDEBUG"
CXX_FLAGS_REL="-O2"

LD_FLAGS="-Wall -Wextra"
LD_FLAGS_DEBUG=""
LD_FLAGS_REL="-flto"
LD_LIBS="-lGL -ldl -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -pthread -lSDL2"

CXX_FLAGS_LIB="$CXX_FLAGS $CXX_FLAGS_REL"

while getopts "dr" ARG; do
    case $ARG in
    d)
        echo "[Debug]"
        CXX_FLAGS="$CXX_FLAGS $CXX_FLAGS_DEBUG"
		LD_FLAGS="$LD_FLAGS_DEBUG"
		break
        ;;
    r)
        echo "[Release]"
        CXX_FLAGS="$CXX_FLAGS $CXX_FLAGS_REL"
        LD_FLAGS="$LD_FLAGS $LD_FLAGS_REL"
	    break
        ;;
    esac
done

mkdir -p build/
pushd build/ > /dev/null

rm -rf *.o

echo "CXX main.cpp"
$CXX -c $CXX_FLAGS ../src/main.cpp -o main.cpp.o

if [ ! $? -eq 0 ]
then
	exit 1
fi

echo "CC sha1.c"
$CC -c $CXX_FLAGS_LIB ../src/sha1.c -o sha1.c.o

echo "CC gl3w.c"
$CC -c $CXX_FLAGS_LIB ../src/gl3w.c -o glfw.c.o

for i in ../src/imgui*.cpp; do
    [ -f "$i" ] || break
    echo "CXX $(basename $i)"
	$CXX -c $CXX_FLAGS_LIB $i -o $(basename $i).o
done

echo -n "Linking "
for i in ./*.o; do
	echo -n $(basename $i)
	echo -n " "
done
echo " "

$LD $LD_FLAGS *.o $LD_LIBS -o PiTerm

echo "Done..."
popd > /dev/null
