#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Error Values
#define SUCCESS    0
#define ERROR      1
#define OUTOFRANGE 2
#define NOMEM      3

// Constants
#define NEG1_8  0xFF
#define NEG1_16 0xFFFF
#define NEG1_32 0xFFFFFFFF
#define NEG1_64 0xFFFFFFFFFFFFFFFF

typedef uint32_t err;
typedef signed char int8;
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef unsigned long uint64;

#define PI	3.14159265358979323846

#define RAW_TYPE  uint8

#endif
