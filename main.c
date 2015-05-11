#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "brain.h"

int main() {
    Params p = {
        .n = 2048,
        .p = 25,
        //.d = 32,
        //.g = 128,
        //.x = 32,
    };
    Layer* l = new_layer(p);
    print(l);
    return 0;
}

void panic(const char* msg) {
    fprintf(stderr, "%s", msg);
    exit(1);
}


