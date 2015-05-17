#define _GNU_SOURCE // for qsort_r
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "brain.h"

// Based on Numenta's Cortical Learning Algorithm

/*

A layer is composed of cells arrayed in columns.
The layer has two tasks:
     Spatial Pooling
     Temporal Pooling

Components of the layer:
    cells - off, on, or ready
    columns
    synapses - connections between cells

A layer is composed of columns of cells. Each cell has proximal dendrites which connect to input bits, dendritic segments, and distal dendrites which form synapses between the dendritic segments and other cells.

All the cells in a column connect to the same input bits. When all those input bits are active, the column bursts. When a cell activates, it sends out an inhibitory signal to its neighbors, which means that only the fastest cells - the ones with the most active synapses - activate at any one time. Cells which are active together form synapses, which helps them activate quicker next time.

*/

int useheap = 1;

static int maxweight(void* v, int i, int j);
static int cmpweight(const void* ap, const void* bp, void* lp);

// Simple LCRNG, for determinism
unsigned randint(unsigned n) {
    static uint32_t seed = 1;
    seed = seed*0x41C64E6D + 0x6073;
    return (seed>>16) % n;
}

Layer*
new_layer(Params p) {
    int i, j;
    Layer* l = malloc(sizeof(Layer));
    if (l == NULL) {
        return NULL;
    }
    if (p.threshold == 0) {
        p.threshold = 20;
    }
    if (p.active == 0) {
        // Aim for 2% activation
        p.active = p.n / 50;
    }
    if (p.reward == 0) {
        p.reward = 10;
    }
    if (p.penalty == 0) {
        p.penalty = 2;
    }
    if (p.radius == 0) {
        //p.radius = 10;
    }
    l->p = p;
    // TODO check return from calloc
    l->colWeight = calloc((size_t)(p.n),     sizeof l->colWeight[0]);
    l->synWeight = calloc((size_t)(p.n*p.p), sizeof l->synWeight[0]);
    l->colSorted = calloc((size_t)(p.n),     sizeof l->colSorted[0]);
    // Initialize synapses
    for (j = 0; j < p.p; j++) {
        l->synWeight[j] = randint(256u);
        /*
        if (j > p.p/2) {
            l->synWeight[j] = l->synWeight[p.p - j - 1];
        } else {
            l->synWeight[j] = (uint8_t)(255 * j / (p.p/2));
        }
        */
    }
    for (i = 1; i < p.n; i++) {
        memmove(&l->synWeight[i*p.p], l->synWeight, (size_t)(p.p) * sizeof l->synWeight[0]);
    }
    return l;
}

void print(Layer *l) {
    int i;
    printf("[size]: %d\n", (int)(sizeof *l) + l->p.n + l->p.n*l->p.p + 4*l->p.n);
    printf("p.n: %d\n", l->p.n);
    printf("p.p: %d\n", l->p.p);
    printf("p.d: %d\n", l->p.d);
    printf("p.g: %d\n", l->p.g);
    printf("p.x: %d\n", l->p.x);
    printf("p.threshold: %d\n", l->p.threshold);
    printf("p.active: %d\n", l->p.active);
    printf("p.reward: %d\n", (int)(l->p.reward));
    printf("p.penalty: %d\n", (int)(l->p.penalty));
    printf("p.radius: %d\n", (int)(l->p.radius));
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
        panic("input length does not equal number of columns");
    }
    if (Blen(&in) != Blen(out)) {
        panic("input and output lengths do not match");
    }
    for (i = 0; i < l->p.n; i++){
        l->colWeight[i] = 0;
    }
    // For each proximal dendrite, figure out which synapses are active.
    // For each proximal dendrite, compute the weight of the number of active synapses
    // For each column, set the weight to the weight of the most active dendrite.
    syn = l->synWeight;
    int h = l->p.p / 2;
    unsigned t = l->p.threshold;
    for (i = 0; i < l->p.n; i++) {
        for (j = 0; j < l->p.p; j++) {
            //if (Bget(&in, i+j-h) && syn[j] > t) {
            //    l->colWeight[i]++;
            //}

            // Branchless version
            int m = (i+j-h + l->p.n)%l->p.n;
            l->colWeight[i] += Bget(&in, m) & ((t - syn[j])>>8);
        }
        syn += l->p.p;
    }

    int active = 0;
    int threshold = l->p.min;
    for (i = 0; i < l->p.n; i++) {
        l->colSorted[i] = i;
    }
    if (useheap) {
        int len = l->p.n;
        int* top = l->colSorted+len;
        int* p;
        heapify(l->colSorted, len, maxweight, l);
        Bclear(out);
        while (active < l->p.active && len > 0) {
            // Pick the top column, and eliminate all columns near it.
            int k = pop(l->colSorted, len, maxweight, l);
            //printf("[%d] = %d\n", k, l->colWeight[k]);
            len--;
            top--;
            if (l->colWeight[k] < threshold) {
                break;
            }
            // If this column is in range of a higher-weight column, lay low.
            if (l->p.radius != 0) {
                for (p = l->colSorted + l->p.n - 1; p > top; p--) {
                    if (k - l->p.radius <= *p && *p <= k + l->p.radius) {
                        goto inhibit;
                    }
                }
            }
            Bset(out, k);
            active++;
        inhibit:
            continue;
        }
    } else {
        // Sort the columns by weight.
        qsort_r(l->colSorted, (size_t)l->p.n, sizeof l->colSorted[0], cmpweight, l);

        Bclear(out);
        for (i = 0; i < l->p.n && active < l->p.active; i++) {
            // Pick the top column, and eliminate all columns near it.
            int k = l->colSorted[i];
            if (l->colWeight[k] < threshold) {
                break;
            }
            // If this column is in range of a higher-weight column, lay low.
            if (l->p.radius != 0) {
                for (j = 0; j < i; j++) {
                    if (k - l->p.radius <= l->colSorted[j] && l->colSorted[j] <= k + l->p.radius) {
                        goto inhibit2;
                    }
                }
            }
            Bset(out, k);
            active++;
        inhibit2:
            continue;
        }
    }
    //printf("%d %d\n", active, l->p.active);

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

static int maxweight(void* v, int i, int j) {
    Layer* l = v;
    if (!(0 <= i && i < l->p.n)) {
        panic("%d out of range", i);
    }
    if (!(0 <= j && j < l->p.n)) {
        panic("%d out of range", j);
    }
    if (l->colWeight[i] == l->colWeight[j]) {
        return i < j;
    }
    return l->colWeight[i] > l->colWeight[j];
}

static int cmpweight(const void* ap, const void* bp, void* lp) {
    Layer* l = lp;
    const int* a = ap;
    const int* b = bp;
    if (l->colWeight[*a] < l->colWeight[*b]) {
        return 1;
    }
    if (l->colWeight[*a] > l->colWeight[*b]) {
        return -1;
    }
    return *a < *b;
}

int time(Layer *l, Bitvec in, Bitvec *out, Bitvec *predict) {
    return 0;
}
