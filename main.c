#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "brain.h"

enum { N = 512, Z = N/8 };
uint8_t buf[Z*2];

Params params = {
    .n = N,
    .p = 100,
    //.d = 32,
    //.g = 128,
    //.x = 32,
    //.radius = 0,
    .threshold = 20,
};

int Bcount(Bitvec* bv) {
    int i, count;
    count = 0;
    for (i = 0; i < bv->n; i++) {
        count += Bget(bv, i);
    }
    return count;
}

void Bprint(Bitvec* bv, char end) {
    int i, j;
    (void)j;
    for (i = 0; i < bv->n/8; i++) {
        unsigned b = bv->v[i];
        /*
        for (j = 0; j < 8; j++){
            putc('0' + (b>>j&1), stdout);
        }
        */
        putc('0' + !!b, stdout);
        //putc("_CG#A###T#######"[b&15], stdout);
        //putc("_CG#A###T#######"[b>>4&15], stdout);
    }
    if (end) {
        putc(end, stdout);
    }
}

int main() {
    Layer* l4, *l3;
    Bitvec in, out4, out3;
    int i, c;
    params.radius = 10;
    params.active = 20;
    params.min = 10;
    l4 = new_layer(params);
    if (l4 == NULL) {
        panic("out of memory");
    }
    params.p = 200;
    params.min = 5;
    params.radius = 0;
    l3 = new_layer(params);
    if (l3 == NULL) {
        panic("out of memory");
    }
    memset(buf, 0, sizeof buf);
    Binitb(&in, buf, Z);
    Binit(&out4, N);
    Binit(&out3, N);
    for (i = 0, c = getc(stdin); c != EOF; i += 2, c = getc(stdin)) {
        if (i >= Z) {
            i = 0;
        }
        buf[i] = (uint8_t)c;
        buf[i+1] = ~(uint8_t)c;
        //Bprint(&in);

        space(l4, in, &out4);
        printf("%3d ", Bcount(&out4));
        Bprint(&out4, ' ');

        space(l3, out4, &out3);
        printf("%3d ", Bcount(&out3));
        Bprint(&out3, '\n');
    }
    //print(l4);
    return 0;
}

void panic(const char* msg, ...) {
    va_list va;
    va_start(va, msg);
    vfprintf(stderr, msg, va);
    putc('\n', stderr);
    va_end(va);
    exit(1);
}
