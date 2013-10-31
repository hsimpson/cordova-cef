#!/bin/sh
./bootstrap.sh toolset=clang
./b2 toolset=clang variant=debug,release --without-mpi cxxflags="-arch x86_64 -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -stdlib=libc++ -ftemplate-depth=512" linkflags="-stdlib=libc++" link=static -j4 -a stage
