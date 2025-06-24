# ELBAF
**elbaf** is command line tool for compressing/decompressing files. It currently uses Huffman encoding as its compression algorithm - other algorithms will be implemented in the future.

Usage:
```
elbaf [options] inputfile outputfile

Options:
  -x : decompression
```

### Compiling
Requirements: CMake, C++20 compiler
```
mkdir build
cmake -B build .
cmake --build build
```

**elbaf** will by default appear under ```build/bin/elbaf```
