#! /bin/bash 

CC=gcc
CXX=g++
LD=g++

CC_FLAGS="-Wall -Wextra -Wno-int-to-pointer-cast"
CC_FLAGS_DEBUG="-g -O0 -DDEBUG"
CC_FLAGS_REL="-O2"

CXX_FLAGS="$CC_FLAGS"
CXX_FLAGS_DEBUG="$CC_FLAGS_DEBUG"
CXX_FLAGS_REL="$CC_FLAGS_REL"

LD_FLAGS="-Wall -Wextra"
LD_FLAGS_DEBUG=""
LD_FLAGS_REL="-flto"
LD_LIBS="-lncurses"

mkdir -p build/

pushd build/ > /dev/null

rm -rf *.o

set -x

$CXX -c $CXX_FLAGS ../src/main.cpp -o main.o
$LD $LD_FLAGS main.o $LD_LIBS -o PiTerm

set +x

popd > /dev/null
