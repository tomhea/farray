#ifndef O1_O1ARRAY_H
#define O1_O1ARRAY_H


// In-Place Initializable Arrays - https://arxiv.org/abs/1709.08900

#include <iostream>

#pragma pack(1)

template<typename T, int N>
union sizeT {
    T v[N];
    size_t ptr;
};

template<typename T, int N>
struct Block {
    sizeT<T,N> first, second;
};

//template <typename T>
//using initfunc = T(*)(size_t);


// B[i] saves the already i/2 index (the block index)

#define CHAINED_TO(chained,i,k)     size_t k = A[i].first.ptr; \
                                    bool chained = (k != i) && (k < n_blocks) && ((i<b) ^ (k<b)) && (A[k].first.ptr == i);
#define MAKE_CHAIN(i,k)     A[i].first.ptr = k; \
                            A[k].first.ptr = i;
#define INIT_BLOCK(i)   for(int _ = 0; _ < HALF_BLOCK_SIZE; _++) A[i].first.v[_] = A[i].second.v[_] = def;

// lines  6-7  in iwrite
#define ASSIGN_UCA(first,mod,i,v)   if (first) { \
                                        A[i].first.v[mod] = v; \
                                        break_chain(i); \
                                    } else { \
                                        A[i].second.v[mod] = v; \
                                    }
// lines 19-22 in iwrite
#define ASSIGN_WCA(first,mod,i,k,v)     A[first ? k : i].second.v[mod] = v;

#define HALF_BLOCK_SIZE     ((sizeof(size_t)-1)/sizeof(T)+1)
#define BLOCK_SIZE (2*HALF_BLOCK_SIZE)

//log2(size_t)/log2(sizeof(T))


template<typename T>
class O1Array {
    size_t n;
    size_t n_blocks;
    Block<T, HALF_BLOCK_SIZE> *A;
    T def;
    size_t b;

    void break_chain(size_t i);
    size_t extend();
    void fillLast();

public:
    O1Array(T* A, size_t n, T def);
    void init();
    void init(T def_value);

    T read(size_t i);
    void write(size_t i, T v);
};


template<typename T>
void O1Array<T>::break_chain(size_t i) {
    CHAINED_TO(chained,i,k)
    if (chained) A[k].first.ptr = k;
}

template<typename T>
size_t O1Array<T>::extend() {
    CHAINED_TO(chained,b,k)
    b++;
    if (!chained) {
        k = b-1;
    } else {
        A[b-1].first = A[k].second;
    }

    INIT_BLOCK(k)
    break_chain(k);
    return k;
}

template<typename T>
void O1Array<T>::fillLast() {
    for (int i = n_blocks*BLOCK_SIZE; i < n; i++) {
        ((T *) A)[i] = def;
    }
}

template<typename T>
O1Array<T>::O1Array(T* A, size_t n, T def)
        : n(n), n_blocks(n/BLOCK_SIZE), A((Block<T,HALF_BLOCK_SIZE>*)A), def(def), b(0) {
    fillLast();
}

template<typename T>
void O1Array<T>::init() {
    b = 0;
    fillLast();
}

template<typename T>
void O1Array<T>::init(T def_value) {
    def = def_value;
    init();
}


template<typename T>
T O1Array<T>::read(size_t i) {
    if (i >= n_blocks * BLOCK_SIZE) { return (i < n) ? ((T*)A)[i] : def; }
    int mod = i % HALF_BLOCK_SIZE; i /= HALF_BLOCK_SIZE;
    bool first = i%2 == 0; i /= 2;

    CHAINED_TO(chained,i,k)
    if (i < b) {    // UCA
        if ( chained) return def;
        return first ? A[i].first.v [mod] : A[i].second.v[mod];
    } else {        // WCA
        if (!chained) return def;
        return first ? A[k].second.v[mod] : A[i].second.v[mod];
    }
}


template<typename T>
void O1Array<T>::write(size_t i, T v) {
    if (i >= n_blocks * BLOCK_SIZE) { if (i < n) ((T*)A)[i] = v; return; }
    int mod = i % HALF_BLOCK_SIZE; i /= HALF_BLOCK_SIZE;
    bool first = i%2 == 0; i /= 2;

    CHAINED_TO(chained,i,k)
    if (i < b) {    // UCA
        if ( chained) {     // not written
            size_t j = extend();
            if (i == j) {
                ASSIGN_UCA(first, mod, i, v)
            } else {
                A[j] = A[i];
                MAKE_CHAIN(j, k)
                INIT_BLOCK(i)
                ASSIGN_UCA(first, mod, i, v)
            }
        } else {            // already written
            ASSIGN_UCA(first, mod, i, v)
        }
    } else {        // WCA
        if (!chained) {     // not written
            k = extend();
            if (i == k) {
                ASSIGN_UCA(first, mod, i, v)
            } else {
                INIT_BLOCK(i)
                MAKE_CHAIN(k, i)
                ASSIGN_WCA(first, mod, i, k, v)
            }
        } else {            // already written
            ASSIGN_WCA(first, mod, i, k, v)
        }
    }
}


#endif //O1_O1ARRAY_H
