#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "brain.h"

int main() {
    Layer* l = new_layer(2048, 25, 128, 200);
    print(l);
    return 0;
}

void panic(const char* msg) {
    fprintf(stderr, "%s", msg);
    exit(1);
}


