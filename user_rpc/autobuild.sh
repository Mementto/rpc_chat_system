#!/bin/bash

set -e

if [ -d `pwd`/build ]; then
    rm -rf `pwd`/build/*
else
    mkdir `pwd`/build
fi

if [ -d `pwd`/bin ]; then
    echo "bin exist"
else
    mkdir `pwd`/bin
fi

if [ -d `pwd`/log ]; then
    echo "log exist"
else
    mkdir `pwd`/log
fi

cd `pwd`/build &&
    cmake .. &&
    make
cd ..