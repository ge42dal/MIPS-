#!/bin/bash
/opt/homebrew/opt/llvm/bin/clang++ \
  -std=c++23 \
  -stdlib=libc++ \
  -L/opt/homebrew/opt/llvm/lib/c++ \
  -L/opt/homebrew/opt/llvm/lib \
  -I/opt/homebrew/opt/llvm/include/c++/v1 \
  -lc++ -lc++abi \
  -Wno-unused-command-line-argument \
  -g \
  -o build/main \
  src/main.cpp
