#ifndef TYPES_H
#define TYPES_H


#include <stdint.h>
#include <stdio.h>



/*
    - This is a file meant for generalized type defintions we may reuse often

    - You maye see typedefs outside of this file, but usually those are made for a specific class 
    or something
*/


typedef unsigned char UCHAR;   // 1-byte unsigned integer

typedef unsigned int UINT;     // usually 4-byte unsigned integer (platform-dependent)

typedef uint8_t  UINT8;         // 8-bit  (1-byte) unsigned integer
typedef uint16_t UINT16;        // 16-bit (2-byte) unsigned integer
typedef uint32_t UINT32;        // 32-bit (4-byte) unsigned integer
typedef uint64_t UINT64;        // 64-bit (8-byte) unsigned integer


#endif