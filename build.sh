#! /bin/bash 

CC=gcc
CXX=g++
LD=g++

CXX_FLAGS="-Wall -Wextra -Wno-int-to-pointer-cast -I/usr/local/include/SDL2 -D_REENTRANT -DTERM_GUI -DIMGUI_IMPL_OPENGL_LOADER_GL3W"
CXX_FLAGS_DEBUG="-g -O0 -DDEBUG"
CXX_FLAGS_REL="-O2"

LD_FLAGS="-Wall -Wextra"
LD_FLAGS_DEBUG=""
LD_FLAGS_REL="-flto"
LD_LIBS="-lncurses -lGL -ldl -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -pthread -lSDL2"

mkdir -p build/

pushd build/ > /dev/null

rm -rf *.o

echo "CXX main.cpp"
$CXX -c $CXX_FLAGS ../src/main.cpp -o main.cpp.o

if [ ! $? -eq 0 ]
then
	exit 1
fi

echo "CC gl3w.c"
$CC -c $CXX_FLAGS ../src/gl3w.c -o glfw.c.o

for i in ../src/imgui*.cpp; do
    [ -f "$i" ] || break
    echo "CXX $(basename $i)"
	$CXX -c $CXX_FLAGS $i -o $(basename $i).o
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
