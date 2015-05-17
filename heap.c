#include <stdlib.h>
#include <stdint.h>
#include "brain.h"

/* Heap sort */

static int debug = 0;

void verify(int *h, int len, int (*less)(void*, int, int), void* v);

static void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// O(log n)
static void siftdown(int* h, int i, int len, int (*less)(void*, int, int), void *v) {
    int k;
    while (i < len) {
        k = i+i+1;
        if (k+1 < len && less(v, h[k+1], h[k])) {
            k = k+1;
        }
        if (k < len && less(v, h[k], h[i])) {
            swap(&h[i], &h[k]);
        } else {
            break;
        }
        i = k;
    }
}

// Heapify converts an array into a heap. O(n)
void heapify(int* h, int len, int (*less)(void*, int, int), void *v) {
    int i;
    // Build a heap in-place by repairing it from the bottom up.
    // The heap invariant is that each node is smaller than its children.
    // Or stated in concrete terms, a[k] <= a[2k+1] and a[k] <= a[2k+2].
    for (i = len; i > 0; i--) {
        siftdown(h, i-1, len, less, v);
    }
    verify(h, len, less, v);
}

// Pop removes and returns the least element in the heap. O(log n)
int pop(int* h, int len, int (*less)(void*, int, int), void* v) {
    if (len <= 0) {
        return 0;
    }
    swap(&h[0], &h[len-1]);
    siftdown(h, 0, len-1, less, v);
    verify(h, len-1, less, v);
    return h[len-1];
}

void verify(int *h, int len, int (*less)(void*, int, int), void* v) {
    int i;
    if (!debug) {
        return;
    }
    for (i = 0; i < len; i++) {
        if (i+i+1 < len && less(v, h[i+i+1], h[i])) {
            panic("heap invarient violated: h[%d] = %d > h[%d] = %d", i, h[i], i+i+1, h[i+i+1]);
        }
        if (i+i+2 < len && less(v, h[i+i+2], h[i])) {
            panic("heap invarient violated: h[%d] = %d > h[%d] = %d", i, h[i], i+i+2, h[i+i+2]);
        }
    }
}
