// WL_UTILS.C

#include "wl_def.h"
#include "wl_utils.h"

/*
===================
=
= safe_malloc
=
= Wrapper for malloc with a NULL check
=
===================
*/

void *safe_malloc (size_t size, const char *fname, uint32_t line)
{
    void *ptr;

    ptr = malloc(size);

    if (!ptr)
        Quit ("SafeMalloc: Out of memory at %s: line %u",fname,line);

    return ptr;
}

fixed FixedDiv2(fixed a, fixed b)
{
    fixed flo;

    asm volatile(
    ".set noreorder\n\t"
    ".set nomacro\n\t"
    "dsll   %1, %1, 16\n\t"
    "ddiv   $0, %1, %2\n\t"
    "mflo   %0\n\t"
    ".set macro\n\t"
    ".set reorder"
    : "=r" (flo)
    : "r" (a), "r" (b)
    );

    return (fixed) flo;
}

fixed FixedDiv(fixed a, fixed b)
{
    fixed     aa, bb;
    unsigned    c;
    int         sign;

    sign = a^b;

    if (a < 0)
        aa = -a;
    else
        aa = a;

    if (b < 0)
        bb = -b;
    else
        bb = b;

    if ((unsigned)(aa >> 14) >= bb)
    {
        if (sign < 0)
            c = INT32_MIN;
        else
            c = INT32_MAX;
    }
    else
    {
        c = (fixed) FixedDiv2(a, b);
    }

    return c;
}

fixed FixedMul(fixed a, fixed b)
{
    fixed flo;

    asm volatile(
    ".set noreorder\n\t"
    ".set nomacro\n\t"
    "dmult   %1, %2\n\t"
    "mflo    %0\n\t"
    "dsra    %0, %0, 16\n\t"
    ".set macro\n\t"
    ".set reorder"
    : "=r" (flo)
    : "r" (a), "r" (b)
    );

    return (fixed) flo;
}


word READWORD (byte *ptr)
{
    word val = ptr[0] | ptr[1] << 8;

    return val;
}

longword READLONGWORD (byte *ptr)
{
    longword val = ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;

    return val;
}
