#! /bin/bash 

CC=gcc
CXX=g++
LD=g++

CXX_FLAGS="-Wall -Wextra -Wno-int-to-pointer-cast"
CXX_FLAGS_DEBUG="-g -O0 -DDEBUG"
CXX_FLAGS_REL="-O2"

LD_FLAGS="-Wall -Wextra"
LD_FLAGS_DEBUG=""
LD_FLAGS_REL="-flto"
LD_LIBS="-lncurses"

mkdir -p build/

pushd build/ > /dev/null

rm -rf *.o

echo "main.cpp"
$CXX -c $CXX_FLAGS ../src/main.cpp -o main.o
$LD $LD_FLAGS main.o $LD_LIBS -o PiTerm

echo "Done..."
popd > /dev/null
