#ifndef O1_O1ARRAY1BIT_H
#define O1_O1ARRAY1BIT_H

#include <stdexcept>

#pragma pack(1)
using namespace std;

/*
Implementation of "In-Place Initializable Arrays" - https://arxiv.org/abs/1709.08900

 It's an array implementation with the fill(v), read(i), write(i,v) methods - all in O(1) worst time complexity.
 read and write are obvious, and fill sets all of the cells at once.

 The special key in this implementation - is that it takes only 1 bit of redundancy, beyond the given array.
 That bit, called flag, will be given to read & write, and will be returned from the fill & write.
  if flag is true  - the array can be viewed as a normal array - it is fully initialized, and can be accessed normally.
  if flag is false - the array isn't completed yet, and must be accessed with the fill/read/write methods.
 In order to prove that this implementation only uses the array and the 1bit as it's playground:
  each call requests the array, its size, and the flag (assumed to be false if unspecified),
  and doesn't maintain, creates or asks for any data structures besides it. not a single bit more.


changes from the article:
 Bug fixing: the article's "extend" implementation should've used 2b-1 instead of 2b+1.
 A[i].first.p.ptr saves the already i/block_size index (the block index).


 All functions and macros are noted with the upper bound of their read and write accesses (HB is HalfBlock size).
*/




template<typename T, int N>
union sizeT {
    T v[N];
    struct { size_t ptr; size_t b; T def; } p;
};

template<typename T, int N>
struct Block {
    sizeT<T,N> first, second;
};


// r<=2
#define CHAINED_TO(chained,i,k)     size_t k = A[i].first.p.ptr; \
                                    bool chained = (k != i) && (k < NUM_BLOCKS) && ((i<b) ^ (k<b)) && (A[k].first.p.ptr == i);
// w==2
#define MAKE_CHAIN(i,k)     A[i].first.p.ptr = k; \
                            A[k].first.p.ptr = i;
// w==2HB
#define INIT_BLOCK(i)   for(int t = 0; t < HALF_BLOCK_SIZE; t++) A[i].first.v[t] = A[i].second.v[t] = def;

// w==HB
#define HALF_INIT_BLOCK(i)  for(int t = 0; t < HALF_BLOCK_SIZE; t++) A[i].second.v[t] = def;

/// lines  6-7  in iwrite (r,w<=2)
#define ASSIGN_UCA(first,mod,i,v)   if (first) { \
                                        A[i].first.v [mod] = v; \
                                        break_chain(A,n,b,i); \
                                    } else { \
                                        A[i].second.v[mod] = v; \
                                    }
/// lines 19-22 in iwrite (w==1)
#define ASSIGN_WCA(first,mod,i,k,v)     A[first ? k : i].second.v[mod] = v;

#define HALF_BLOCK_SIZE     ((sizeof(size_t)*2+sizeof(T)-1)/sizeof(T)+1)
#define BLOCK_SIZE  (2*HALF_BLOCK_SIZE)
#define NUM_BLOCKS  (n/BLOCK_SIZE)
#define BLOCKS_END  (NUM_BLOCKS * BLOCK_SIZE)

#define B       A[NUM_BLOCKS-1].first.p.b
#define DEF     A[NUM_BLOCKS-1].first.p.def

#define PREP_A      auto A = (Block<T,HALF_BLOCK_SIZE>*)arr;
// r <= 2
#define PREP_B_DEF      size_t b = B; T def = DEF;

#define PREP_INDICES    size_t mod = i % HALF_BLOCK_SIZE; \
                        i /= HALF_BLOCK_SIZE; \
                        bool first = i%2 == 0; \
                        i /= 2;

// r <= 2, w <= 1
template<typename T>
static void break_chain(Block<T,HALF_BLOCK_SIZE>* A, const size_t n, const size_t b, const size_t i) {
    CHAINED_TO(chained,i,k)
    if (chained) A[k].first.p.ptr = k;
}

// r <= 4, w <= 3HB+1
template<typename T>
static size_t extend(Block<T,HALF_BLOCK_SIZE>* A, const size_t n, size_t& b, const T& def) {
    CHAINED_TO(chained,b,k)
    b++;
    if (!chained) {
        k = b-1;
    } else {
        A[b-1].first = A[k].second;
    }

    INIT_BLOCK(k)
    break_chain(A, n, b, k);
    return k;
}


// w <= 2HB+1
template<typename T>
bool fill(T* arr, size_t n, T v) {
    for (size_t i = BLOCKS_END; i < n; i++) arr[i] = v;
    if (NUM_BLOCKS == 0) return true;
    PREP_A
    B = 0;
    DEF = v;
    return false;
}


// r <= 5
template<typename T>
T read(const T* arr, size_t n, size_t i, bool flag = false) {
    if (flag || i >= BLOCKS_END) {      // arr functions as regular array
        if (i < n) return arr[i];
        throw out_of_range("Reading from index " + to_string(i) + " (>=" + to_string(n) + ")");
    }

    PREP_A
    PREP_B_DEF
    PREP_INDICES

    CHAINED_TO(chained,i,k)
    if (i < b) {    // UCA
        if ( chained) return def;
        return first ? A[i].first.v [mod] : A[i].second.v[mod];
    } else {        // WCA
        if (!chained) return def;
        return first ? A[k].second.v[mod] : A[i].second.v[mod];
    }
}


// already_written: r <= 6, w <= 2
//     first_write: r <= 6, w <= 5HB+5
template<typename T>
bool write(T* arr, size_t n, size_t i, const T& v, bool flag = false) {
    if (flag || i >= BLOCKS_END) {      // arr functions as regular array
        if (i < n) { arr[i] = v; return flag; }
        throw out_of_range("Writing to index " + to_string(i) + " (>=" + to_string(n) + ")");
    }

    PREP_A
    PREP_B_DEF
    PREP_INDICES

    CHAINED_TO(chained,i,k)
    if (i < b) {    // UCA
        if ( chained) {     // not written
            size_t j = extend(A,n,b,def);
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
            k = extend(A,n,b,def);
            if (i == k) {
                ASSIGN_UCA(first, mod, i, v)
            } else {
                HALF_INIT_BLOCK(i)
                MAKE_CHAIN(k, i)
                ASSIGN_WCA(first, mod, i, k, v)
            }
        } else {            // already written
            ASSIGN_WCA(first, mod, i, k, v)
        }
    }

    if (b == NUM_BLOCKS) return true;
    B = b;
    return false;
}


#endif //O1_O1ARRAY1BIT_H
