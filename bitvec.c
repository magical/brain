#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "brain.h"

void
Binit(Bitvec* bv, int n)
{
    int size = _Bsize(n);
    if (size == 0) {
        goto error;
    }
    bv->n = n;
    bv->v = malloc((size_t)(size));
    if (bv->v == NULL) {
        goto error;
    }
    memset(bv->v, 0, (size_t)(size));
    return;
error:
    bv->n = 0;
    bv->v = NULL;
}

void
Binitb(Bitvec* bv, uint8_t* buf, size_t size)
{
    bv->v = buf;
    bv->n = (int)size * 8;
}

int
_Bsize(int n)
{
    int size;
    if (n < 0) {
        return 0;
    }
    size = (n+7) / 8;
    if (size < 0) {
        return 0;
    }
    if (size == 0) {
        return 1;
    }
    return size;
}

uint8_t
Bget(Bitvec* bv, int index)
{
    if (index < 0 || bv->n <= index) {
        return 0;
    }
    uint8_t v = bv->v[(unsigned)index/8];
    return (v >> (unsigned)index%8) & 1;
}

void
Bset(Bitvec* bv, int index)
{
    if (index < 0 || bv->n < index) {
        return;
    }
    bv->v[(unsigned)index/8] |= (uint8_t)(1 << (unsigned)index%8);
}

void
Bclear(Bitvec* bv)
{
    int size = _Bsize(bv->n);
    if (size == 0) {
        return;
    }
    memset(bv->v, 0, (size_t)(size));
}
