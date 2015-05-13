#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "brain.h"

enum { N = 512, Z = N/8 };
uint8_t buf[Z+Z];

Params params = {
    .n = N,
    .p = 51,
    //.d = 32,
    //.g = 128,
    //.x = 32,
    .radius = 10,
};

int Bcount(Bitvec* bv) {
    int i, count;
    for (i = 0; i < bv->n; i++) {
        count += Bget(bv, i);
    }
    return count;
}

void Bprint(Bitvec* bv) {
    int i, j;
    for (i = 0; i < bv->n/8; i++) {
        unsigned b = bv->v[i];
        for (j = 0; j < 8; j++){
            putc('0' + (b>>j&1), stdout);
        }
    }
    putc('\n', stdout);
}

int main() {
    Layer* l;
    Bitvec in;
    Bitvec out;
    int i, c;
    l = new_layer(params);
    if (l == NULL) {
        panic("out of memory");
    }
    memset(buf, 0, sizeof buf);
    Binit(&out, N);
    for (i = 0, c = getc(stdin); c != EOF; i++, c = getc(stdin)) {
        if (i > Z) {
            memmove(buf, buf+i, sizeof buf - (size_t)(i));
            i = 0;
        }
        buf[i+Z-1] = (uint8_t)c;
        if (1 || i % 10 == 0) {
            Binitb(&in, buf+i, Z);
            //Bprint(&in);
            space(l, in, &out);
            printf("%3d ", Bcount(&out));
            Bprint(&out);
        }
    }
    //print(l);
    return 0;
}

void panic(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}
