# ELBAF
**elbaf** is a command line tool for compressing/decompressing files. It uses Huffman encoding as its compression algorithm.

Usage:
```
elbaf [options] inputfile outputfile

Options:
  -x : decompression
```


### Compiling
Requirements: CMake, C++20 compiler, build tool (make, ninja, ...)
```
mkdir build
cmake -B build .
cmake --build build
```

**elbaf** will by default appear under ```build/bin/elbaf```


### Header layout
- 1 bytes: the number of symbols in the encoding table
- N bytes: the symbols ordered from most used to least used (1 byte each)
- 4 bytes: the file size in network byte order
- x bytes: the encoded file
