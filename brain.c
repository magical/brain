#define _GNU_SOURCE // for qsort_r
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "brain.h"

// Based on Numenta's Corticial Learning Algorithm

/*

A layer is composed of cells arrayed in columns.
The layer has two tasks:
     Spacial Pooling
     Temporal Pooling

Components of the layer:
    cells - off, on, or ready
    columns
    synapses - connections between cells

A layer is composed of columns of cells. Each cell has proximal dendrites which connect to input bits, dendritic segments, and distal dendrites which form synapses between the dendritic segments and other cells.

All the cells in a column connect to the same input bits. When all those input bits are active, the column bursts. When a cell activates, it sends out an inhibitory signal to its neighbors, which means that only the fastest cells - the ones with the most active synapses - activate at any one time. Cells which are active together form synapses, which helps them activate quicker next time.

*/

int cmpweight(const void* ap, const void* bp, void* lp);

Layer*
new_layer(Params p) {
    int i, j;
    Layer* l = malloc(sizeof(Layer));
    if (l == NULL) {
        return NULL;
    }
    if (p.threshold == 0) {
        p.threshold = 128;
    }
    if (p.active == 0) {
        // Aim for 2% activation
        p.active = p.n / 50;
    }
    if (p.reward == 0) {
        p.reward = 10;
    }
    if (p.penalty == 0) {
        p.penalty = 5;
    }
    l->p = p;
    l->colWeight = calloc((size_t)(p.n),     sizeof l->colWeight[0]);
    l->synWeight = calloc((size_t)(p.n*p.p), sizeof l->synWeight[0]);
    l->colSorted = calloc((size_t)(p.n),     sizeof l->colSorted[0]);
    for (j = 0; j < p.p; j++) {
        if (j > p.p/2) {
            l->synWeight[j] = l->synWeight[p.p - j - 1];
        } else {
            l->synWeight[j] = (uint8_t)(255 * j / (p.p/2));
        }
    }
    for (i = 1; i < p.n; i++) {
        memmove(&l->synWeight[i*p.p], l->synWeight, (size_t)(p.p) * sizeof l->synWeight[0]);
    }
    return l;
}

void print(Layer *l) {
    int i;
    printf("p.n: %d\n", l->p.n);
    printf("p.p: %d\n", l->p.p);
    printf("p.d: %d\n", l->p.d);
    printf("p.g: %d\n", l->p.g);
    printf("p.x: %d\n", l->p.x);
    printf("p.threshold: %d\n", l->p.threshold);
    printf("p.active: %d\n", l->p.active);
    printf("p.reward: %d\n", (int)(l->p.reward));
    printf("p.penalty: %d\n", (int)(l->p.penalty));
    printf("colWeight: [");
    for (i = 0; i < l->p.n; i++) {
        printf("%d ", l->colWeight[i]);
    }
    printf("]\n");
    printf("synWeight: [");
    for (i = 0; i < l->p.n*l->p.p; i++) {
        printf("%d ", l->synWeight[i]);
    }
    printf("]\n");
    printf("colSorted: [");
    for (i = 0; i < l->p.n; i++) {
        printf("%d ", l->colSorted[i]);
    }
    printf("]\n");
}

int space(Layer *l, Bitvec in, Bitvec *out) {
    uint8_t *syn;
    int i, j;
    if (l->p.n != Blen(&in)) {
        panic("input length does not equal column length");
    }
    for (i = 0; i < l->p.n; i++){
        l->colWeight[i] = 0;
    }
    // For each proximal dendrite, figure out which synapses are active.
    // For each proximal dendrite, compute the weight of the number of active synapses
    // For each column, set the weight to the weight of the most active dendrite.
    syn = l->synWeight;
    for (i = 0; i < l->p.n; i++) {
        for (j = 0; j < l->p.p; j++) {
            // TODO: branchless
            if (Bget(&in, i + j - l->p.p/2) && syn[j] > l->p.threshold) {
                l->colWeight[i]++;
            }
        }
        syn += l->p.p;
    }

    // Sort the columns by weight.
    for (i = 0; i < l->p.n; i++) {
        l->colSorted[i] = i;
    }
    qsort_r(l->colSorted, (size_t)l->p.n, sizeof l->colSorted[0], cmpweight, l);

    // Pick the top column, and eliminate all columns near it.
    Bclear(out);
    for (i = 0; i < l->p.active; i++) {
        Bset(out, i);
    }

    // For each proximal dendrite, if its cell is active, strengthen the connections of the synapses corresponding to active inputs. If inactive, weaken the connections of synapses corresponding to inactive inputs.
    syn = l->synWeight;
    for (i = 0; i < l->p.n; i++) {
        if (Bget(out, i)) {
            for (j = 0; j < l->p.p; j++) {
                if (Bget(&in, i + j - l->p.p/2)) {
                    syn[j] = saddu8(syn[j], l->p.reward);
                }
            }
        } else {
            for (j = 0; j < l->p.p; j++) {
                if (!Bget(&in, i + j - l->p.p/2)) {
                    syn[j] = ssubu8(syn[j], l->p.penalty);
                }
            }
        }
        syn += l->p.p;
    }

    return 0;
}

int cmpweight(const void* ap, const void* bp, void* lp) {
    Layer* l = lp;
    const int* a = ap;
    const int* b = bp;
    if (l->colWeight[*a] < l->colWeight[*b]) {
        return -1;
    }
    if (l->colWeight[*a] > l->colWeight[*b]) {
        return 1;
    }
    return 0;
}

int time(Layer *l, Bitvec in, Bitvec *out, Bitvec *predict) {
    return 0;
}
