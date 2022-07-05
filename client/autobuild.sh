#!/bin/bash

set -e

if [ -d `pwd`/build ]; then
    rm -rf `pwd`/build/*
else
    mkdir `pwd`/build
fi

cd `pwd`/build &&
    cmake .. &&
    make
cd ..