#!/bin/bash
set -e
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

if [ ! -d /usr/include/tiny_muduo ]; then
    mkdir /usr/include/tiny_muduo
fi
for header in `ls *.h`
do
    cp $header /usr/include/tiny_muduo
done

cp `pwd`/lib/libtiny_muduo.so /usr/lib


ldconfig

