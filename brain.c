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

// A dense bit vector
typedef struct Bitvec Bitvec;

// A sparse bit vector
typedef struct Bitset Bitset;

struct Layer {
    // params
    int n, p, d, g, x;
    int threshold;
    int active;
    uint8_t reward;
    uint8_t penalty;
    // N columns
    // N proximal dendrites
    // N*P column synapses
    // N*D cells
    // N*D*G segments
    // N*D*G*X dendrites

    uint8_t* colWeight; // N
    uint8_t* synWeight; // N*P
    int* colSorted; // N
};

int cmpweight(const void* ap, const void* bp, void* lp);

Layer*
new_layer(int n, int p, int threshold, int active) {
    int i, j;
    Layer* l = malloc(sizeof(Layer));
    if (l == NULL) {
        return NULL;
    }
    l->n = n;
    l->p = p;
    l->d = 0;
    l->g = 0;
    l->x = 0;
    l->threshold = threshold;
    l->active = active;
    l->reward = 10;
    l->penalty = 5;
    l->colWeight = calloc((size_t)(n),   sizeof l->colWeight[0]);
    l->synWeight = calloc((size_t)(n*p), sizeof l->synWeight[0]);
    l->colSorted = calloc((size_t)(n),   sizeof l->colSorted[0]);
    for (j = 0; j < p; j++) {
        if (j > p/2) {
            l->synWeight[j] = l->synWeight[p - j - 1];
        } else {
            l->synWeight[j] = (uint8_t)(255 * j / (p/2));
        }
    }
    for (i = 1; i < n; i++) {
        memmove(&l->synWeight[i*p], l->synWeight, (size_t)(p) * sizeof l->synWeight[0]);
    }
    return l;
}

void print(Layer *l) {
    int i;
    printf("n: %d\n", l->n);
    printf("p: %d\n", l->p);
    printf("d: %d\n", l->d);
    printf("g: %d\n", l->g);
    printf("x: %d\n", l->x);
    printf("threshold: %d\n", l->threshold);
    printf("active: %d\n", l->active);
    printf("reward: %d\n", (int)(l->reward));
    printf("penalty: %d\n", (int)(l->penalty));
    printf("colWeight: [");
    for (i = 0; i < l->n; i++) {
        printf("%d ", l->colWeight[i]);
    }
    printf("]\n");
    printf("synWeight: [");
    for (i = 0; i < l->n*l->p; i++) {
        printf("%d ", l->synWeight[i]);
    }
    printf("]\n");
    printf("colSorted: [");
    for (i = 0; i < l->n; i++) {
        printf("%d ", l->colSorted[i]);
    }
    printf("]\n");
}

int space(Layer *l, Bitvec in, Bitvec *out) {
    uint8_t *syn;
    int i, j;
    if (l->n != Blen(&in)) {
        panic("input length does not equal column length");
    }
    for (i = 0; i < l->n; i++){
        l->colWeight[i] = 0;
    }
    // For each proximal dendrite, figure out which synapses are active.
    // For each proximal dendrite, compute the weight of the number of active synapses
    // For each column, set the weight to the weight of the most active dendrite.
    syn = l->synWeight;
    for (i = 0; i < l->n; i++) {
        for (j = 0; j < l->p; j++) {
            // TODO: branchless
            if (Bget(&in, i + j - l->p/2) && (int)syn[j] > l->threshold) {
                l->colWeight[i]++;
            }
        }
        syn += l->p;
    }

    // Sort the columns by weight.
    for (i = 0; i < l->n; i++) {
        l->colSorted[i] = i;
    }
    qsort_r(l->colSorted, (size_t)l->n, sizeof l->colSorted[0], cmpweight, l);

    // Pick the top column, and eliminate all columns near it.
    Bclear(out);
    for (i = 0; i < l->active; i++) {
        Bset(out, i);
    }

    // For each proximal dendrite, if its cell is active, strengthen the connections of the synapses corresponding to active inputs. If inactive, weaken the connections of synapses corresponding to inactive inputs.
    syn = l->synWeight;
    for (i = 0; i < l->n; i++) {
        if (Bget(out, i)) {
            for (j = 0; j < l->p; j++) {
                if (Bget(&in, i + j - l->p/2)) {
                    syn[j] = saddu8(syn[j], l->reward);
                }
            }
        } else {
            for (j = 0; j < l->p; j++) {
                if (!Bget(&in, i + j - l->p/2)) {
                    syn[j] = ssubu8(syn[j], l->penalty);
                }
            }
        }
        syn += l->p;
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
