//#include <stdint.h>

/*bit vectors*/

typedef struct Bitvec Bitvec;
struct Bitvec {
    uint8_t* v;
    int n; // # bits
};

void    Binit(Bitvec*, int size);
//int     Blen(Bitvec*);
int     _Bsize(int n);
uint8_t Bget(Bitvec*, int index);
void    Bset(Bitvec*, int index);
void    Bclear(Bitvec*);

static inline int
Blen(Bitvec* bv) {
    return bv->n;
}

/* panic */
void panic(const char* msg);


/* saturating arithmetic */

static inline uint8_t
saddu8(uint8_t a, uint8_t b)
{
    uint32_t c = (uint32_t)(a) + (uint32_t)(b);
    return (uint8_t)(c | -(c>>8));
}

static inline uint8_t
ssubu8(uint8_t a, uint8_t b)
{
    uint32_t c = (uint32_t)(a) + (uint32_t)(b);
    return (uint8_t)(c & ~(c>>8));
}

/* brain */

typedef struct Layer Layer;

Layer* new_layer(int n, int p, int threshold, int active);
void print(Layer *l);
