#! /bin/bash

CC=gcc
CXX=g++
LD=g++

CC_FLAGS="-Wall -Wextra -Werror"
CC_FLAGS_DEBUG="-g -O0 -DDEBUG"
CC_FLAGS_REL="-O2"

CXX_FLAGS="$CC_FLAGS"
CXX_FLAGS_DEBUG="$CC_FLAGS_DEBUG"
CXX_FLAGS_REL="$CXX_FLAGS_REL"

LD_FLAGS="-Wall -Wextra -Werror"
LD_FLAGS_DEBUG=""
LD_FLAGS_REL="-flto"

mkdir -p build/

pushd build/ > /dev/null

rm -rf *.o

for i in ../src/*.c; do
    [ -f "$i" ] || break

    echo "$CC -c $CC_FLAGS $i -o $(basename $i).o"
    $CC -c $CC_FLAGS $i -o $(basename $i).o
done

for i in ../src/*.cpp; do
    [ -f "$i" ] || break

    echo "$CXX -c $CXX_FLAGS $i -o $(basename $i).o"
    $CXX -c $CXX_FLAGS $i -o $(basename $i).o
done

echo "$LD $LD_FLAGS *.o -o PiTerm"
$LD $LD_FLAGS *.o -o PiTerm

popd > /dev/null
