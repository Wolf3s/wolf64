#ifndef __WL_TYPES_H_
#define __WL_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef int32_t fixed;
typedef uint32_t longword;
typedef int8_t boolean;

typedef struct
{
    int x,y;
} Point;

typedef struct
{
    Point ul,lr;
} Rect;

#endif
