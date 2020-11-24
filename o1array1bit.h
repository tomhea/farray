#ifndef O1_O1ARRAY1BIT_H
#define O1_O1ARRAY1BIT_H

#include <stdexcept>
#include <iostream>


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



#pragma pack(push, 1)
template<typename T>
struct ptrBdef { size_t ptr; size_t b; T def; };

template<typename T, int N>
union sizeT {
    T v[N];
    ptrBdef<T> p;
};

template<typename T, int N>
struct Block {
    sizeT<T,N> first, second;
};
#pragma pack(pop)


namespace defines {
    template<typename T>
    constexpr size_t halfBlockSize() {
        return ((sizeof(size_t)*2+sizeof(T)-1)/sizeof(T)+1);
    }
    template<typename T>
    constexpr size_t blockSize() {
        return 2*halfBlockSize<T>();
    }

    template<typename T>
    class ArrayHelper {
    public:
        Block<T,halfBlockSize<T>()>* A;
        size_t n;

        ArrayHelper(T* A, size_t n) : A((Block<T,halfBlockSize<T>()>*)A), n(n) {}
        size_t numBlocks() { return n / blockSize<T>(); }
        size_t blocksEnd() { return numBlocks() * blockSize<T>(); }

        ptrBdef<T>& lastP() { return A[numBlocks()-1].first.p; }
        size_t B() { return lastP().b; }    // r==1
        T    DEF() { return lastP().def; }  // r==1
        void expendB() {    lastP().b++; }  // r,w==1
        void fillBottom(T v) { ptrBdef<T>& p = lastP(); p.b = 0; p.def = v; }   // w==2

        void setIndices(size_t& i, size_t& mod, bool& first) {
            size_t hbs = halfBlockSize<T>();
            mod = i % hbs;
            i /= hbs;
            first = (i%2 == 0);
            i /= 2;
        }
        // r<=2
        bool chainedTo(size_t i, size_t& k) {
            k = A[i].first.p.ptr;
            size_t b = B();
            return (k != i) && (k < numBlocks()) && ((i<b) ^ (k<b))
                && (A[k].first.p.ptr == i);
        }
        // w==2
        void makeChain(size_t i, size_t& k) {
            A[i].first.p.ptr = k;
            A[k].first.p.ptr = i;
        }

        // w==2HB
        void initBlock(size_t i) {
            T def = DEF();
            for(int t = 0; t < halfBlockSize<T>(); t++)
                A[i].first.v[t] = A[i].second.v[t] = def;
        }
        // w==HB
        void halfInitBlock(size_t i) {
            T def = DEF();
            for(int t = 0; t < halfBlockSize<T>(); t++)
                A[i].second.v[t] = def;
        }

        // r <= 2, w <= 1
        void breakChain(size_t i) {
            size_t k;
            if (chainedTo(i, k))
                A[k].first.p.ptr = k;
        }

        // r <= 5, w <= 3HB+3
        size_t extend() {
            size_t oldB = B(), k;
            bool chained = chainedTo(oldB, k);
            expendB();
            if (!chained) {
                k = oldB;
            } else {
                A[oldB].first = A[k].second;
                breakChain(oldB);
            }
            initBlock(k);
            breakChain(k);
            return k;
        }
        // r==1
        T read(size_t i, size_t mod, bool first) const {
            return first ? A[i].first.v[mod] : A[i].second.v[mod];
        }

        /// lines  6-7  in iwrite (r,w<=2)
        void write(size_t i, size_t mod, bool first, const T& v) {
            if (first) {
                A[i].first.v [mod] = v;
                breakChain(i);
            } else {
                A[i].second.v[mod] = v;
            }
        }

        bool flag() { return B() == numBlocks(); }
    };
}


// w <= 2HB+1
template<typename T>
bool fill(T* arr, size_t n, T v) {
    defines::ArrayHelper<T> h(arr, n);
    for (size_t i = h.blocksEnd(); i < n; i++) arr[i] = v;
    if (h.numBlocks() == 0) return true;
    h.fillBottom(v);
    return false;
}


// r <= 4
template<typename T>
T read(T* arr, size_t n, size_t i, bool flag = false) {
    defines::ArrayHelper<T> h(arr, n);
    if (flag || i >= h.blocksEnd()) {      // arr functions as regular array
        if (i < n) return arr[i];
        throw std::out_of_range("Reading from index " + std::to_string(i) + " (>=" + std::to_string(n) + ")");
    }
    size_t b = h.B(), mod, k;
    bool first, chained;
    h.setIndices(i, mod, first);
    chained = h.chainedTo(i, k);

    if (i < b) {    // UCA
        if ( chained) return h.DEF();
        return h.read(i, mod, first);
    } else {        // WCA
        if (!chained) return h.DEF();
        return h.read(first ? k : i, mod, false);
    }
}


// already_written: r <= 5, w <= 2
//     first_write: r <= 9, w <= 6HB+7
template<typename T>
bool write(T* arr, size_t n, size_t i, const T& v, bool flag = false) {
    defines::ArrayHelper<T> h(arr, n);
    if (flag || i >= h.blocksEnd()) {      // arr functions as regular array
        if (i < n) { arr[i] = v; return flag; }
        throw std::out_of_range("Writing to index " + std::to_string(i) + " (>=" + std::to_string(n) + ")");
    }

    size_t b = h.B(), mod, k;
    bool first, chained;
    h.setIndices(i, mod, first);
    chained = h.chainedTo(i, k);

    if (i < b) {    // UCA
        if ( chained) {     // not written
            size_t j = h.extend();
            if (i == j) {
                h.write(i, mod, first, v);
            } else {
                h.A[j].second = h.A[i].second;
                h.makeChain(j, k);
                h.initBlock(i);
                h.write(i, mod, first, v);
            }
        } else {            // already written
            h.write(i, mod, first, v);
        }
    } else {        // WCA
        if (!chained) {     // not written
            k = h.extend();
            if (i == k) {
                h.write(i, mod, first, v);
            } else {
                h.halfInitBlock(i);
                h.makeChain(k, i);
                h.write(first ? k : i, mod, false, v);
            }
        } else {            // already written
            h.write(first ? k : i, mod, false, v);
        }
    }

    return h.flag();
}


template<typename T>
size_t writtenSize(T* arr, size_t n, bool flag = false) {
    if (flag) return n;
    defines::ArrayHelper<T> h(arr, n);
    return h.B() * defines::blockSize<T>() + (n - h.blocksEnd());
}

#endif //O1_O1ARRAY1BIT_H