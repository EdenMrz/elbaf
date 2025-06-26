# ELBAF
**elbaf** is a command line tool for compressing/decompressing files. It currently uses Huffman encoding as its compression algorithm - other algorithms will be implemented in the future.

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


### Limitations
The file to be compressed has to be 255 bytes maximum. This is due to the fact that the header in the compressed file stores the file size in one byte.
