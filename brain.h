//#include <stdint.h>

/*bit vectors*/

typedef struct Bitvec Bitvec;

struct Bitvec {
    uint8_t* v;
    int n; // # bits
};

void    Binit(Bitvec*, int n);
void    Binitb(Bitvec*, uint8_t*, size_t size);
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

typedef struct Params Params;
typedef struct Layer Layer;

struct Params {
    // N columns
    // N proximal dendrites
    // N*P column synapses
    // N*D cells
    // N*D*G segments
    // N*D*G*X dendrites
    int n, p, d, g, x;

    uint8_t threshold; // the point at which a synapse is active
    int active; // the number of active columns in output
    uint8_t reward; // amount to strengthen active synapse
    uint8_t penalty; // amount to weaken unused synapses
    int radius; // inhibition radius
};

struct Layer {
    Params p;
    uint8_t* colWeight; // N
    uint8_t* synWeight; // N x P
    int*     colSorted; // N
};

Layer* new_layer(Params p);
void print(Layer *l);
int space(Layer *l, Bitvec in, Bitvec *out);
