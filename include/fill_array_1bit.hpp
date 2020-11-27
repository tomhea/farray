#ifndef O1_FILL_ARRAY_1BIT_H
#define O1_FILL_ARRAY_1BIT_H

#include <stdexcept>
#include <iostream>
#include <stdbool.h>


/*
Implementation of "In-Place Initializable Arrays" paper - https://arxiv.org/abs/1709.08900
 It's an array implementation with the fill(v), read(i), write(i,v) methods - all in O(1) worst time complexity.
 read and write are obvious, and fill sets all of the cells at once.
 The special key in this implementation - is that it takes only 1 bit of redundancy, beyond the given array.
 That bit, called flag, will be given to read & write, and will be returned from the fill & write.
  if flag is true  - the array can be viewed as a normal array - it is fully initialized, and can be accessed normally.
  if flag is false - the array isn't completed yet, and must be accessed with the fill/read/write methods.
 In order to prove that this implementation only uses the array and the 1bit as it's playground:
  each call requests the array, its size, and the flag (assumed to be false if unspecified),
  and doesn't maintain any other data structure between calls. not a single bit more.
  It does use the ArrayHelper struct - but it's created statically and disappears after the call is over.
 The Holder class is for a simple use of the fill array.
  It does only save the array pointer and the flag as non-constants fields.
changes from the article:
 Bug fixing: the article's "extend" implementation should've used 2b-1 instead of 2b+1.
 A[i].first.p.ptr saves the already i/block_size index (the block index).
 All functions and macros are noted with the upper bound of their read and write accesses (HB is HalfBlock size).
*/


namespace FillArray {
    namespace defines {
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

        template<typename T>
        constexpr size_t halfBlockSize() {
            return ((sizeof(size_t)*2+sizeof(T)-1)/sizeof(T)+1);
        }
        template<typename T>
        constexpr size_t blockSize() {
            return 2*halfBlockSize<T>();
        }


        template<typename T>
        struct ArrayHelper {
        public:
            Block<T,halfBlockSize<T>()>* A;
            size_t n, b;
            bool flag;
            T def;

            // r <= 2
            ArrayHelper(const T* A, size_t n, bool flag = true) : A((Block<T,halfBlockSize<T>()>*)A), n(n), flag(flag) {
                if (!flag) {
                    auto p = lastP();
                    b = p.b;
                    def = p.def;
                }
            }
            size_t numBlocks() const { return n / blockSize<T>(); }
            size_t blocksEnd() const { return numBlocks() * blockSize<T>(); }

            ptrBdef<T>& lastP() { return A[numBlocks()-1].first.p; }
            void expendB() { flag = ((b=++lastP().b) == numBlocks()); }  // r == 1, w == 1
            void fillBottom(const T& v) { ptrBdef<T>& p = lastP(); p.b = 0; p.def = v; }   // w == 2

            void setIndices(size_t& i, size_t& mod, bool& first) const {
                size_t hbs = halfBlockSize<T>();
                mod = i % hbs;
                i /= hbs;
                first = (i%2 == 0);
                i /= 2;
            }

            // r == 2
            bool chainedTo(size_t i, size_t& k) const {
                k = A[i].first.p.ptr;
                return (k != i) && (k < numBlocks()) && ((i<b) ^ (k<b))
                    && (A[k].first.p.ptr == i);
            }
            // w == 2
            void makeChain(size_t i, size_t& k) {
                A[i].first.p.ptr = k;
                A[k].first.p.ptr = i;
            }

            // r <= 2, w <= 2HB+1
            void initBlock(size_t i) {
                 firstHalfInitBlock(i);
                secondHalfInitBlock(i);
            }
            // r <= 2, w <= HB+1
            void firstHalfInitBlock(size_t i) {
                for (int t = 0; t < halfBlockSize<T>(); t++)
                    A[i].first.v[t] = def;
                breakChain(i);
            }
            // w == HB
            void secondHalfInitBlock(size_t i) {
                for (int t = 0; t < halfBlockSize<T>(); t++)
                    A[i].second.v[t] = def;
            }

            // r <= 2, w <= 1
            void breakChain(size_t i) {
                size_t k;
                if (chainedTo(i, k))
                    A[k].first.p.ptr = k;
            }

            // r <= 5, w <= 2HB+2
            size_t extend() {
                size_t k;
                bool chained = chainedTo(b, k);
                expendB();
                if (!chained) {
                    k = b-1;
                } else {
                    A[b-1].first = A[k].second;
                    breakChain(b-1);
                }
                secondHalfInitBlock(k);
                return k;
            }

            // r == 1
            T read(size_t i, size_t mod, bool first) const {
                return first ? A[i].first.v[mod] : A[i].second.v[mod];
            }

            /// lines  6-7  in iwrite (r <= 2, w <= 2)
            void write(size_t i, size_t mod, bool first, const T& v) {
                if (first) {
                    A[i].first.v [mod] = v;
                    breakChain(i);
                } else {
                    A[i].second.v[mod] = v;
                }
            }

            // w == HB
            void copySecondHalfBlock(size_t to, size_t from) { A[to].second = A[from].second; }
        };
    }


    // w <= 2HB+1
    template<typename T>
    bool fill(T* A, size_t n, const T& v) {
        defines::ArrayHelper<T> h(A, n);
        for (size_t i = h.blocksEnd(); i < n; i++) A[i] = v;
        if (h.numBlocks() == 0) return true;
        h.fillBottom(v);
        return false;
    }


    // r <= 5
    template<typename T>
    T read(const T* A, size_t n, size_t i, bool flag = false) {
        defines::ArrayHelper<T> h(A, n, flag);
        if (flag || i >= h.blocksEnd()) {      // arr functions as regular array
            if (i < n) return A[i];
            throw std::out_of_range("Reading from index " + std::to_string(i) + " (>=" + std::to_string(n) + ")");
        }
        size_t mod, k;
        bool first, chained;
        h.setIndices(i, mod, first);
        chained = h.chainedTo(i, k);

        if (i < h.b) {  // UCA
            if ( chained) return h.def;
            return h.read(i, mod, first);
        } else {        // WCA
            if (!chained) return h.def;
            return h.read(first ? k : i, mod, false);
        }
    }


    // already_written: r <=  6, w <=     2         (or another index in the same block was written)
    //     first_write: r <= 13, w <= 5HB+7
    template<typename T>
    bool write(T* A, size_t n, size_t i, const T& v, bool flag = false) {
        defines::ArrayHelper<T> h(A, n, flag);
        if (flag || i >= h.blocksEnd()) {      // arr functions as regular array
            if (i < n) { A[i] = v; return flag; }
            throw std::out_of_range("Writing to index " + std::to_string(i) + " (>=" + std::to_string(n) + ")");
        }

        size_t mod, k;
        bool first, chained;
        h.setIndices(i, mod, first);
        chained = h.chainedTo(i, k);

        if (i < h.b) {    // UCA
            if ( chained) {     // not written
                size_t j = h.extend();
                if (i == j) {
                    h.firstHalfInitBlock(i);
                    h.write(i, mod, first, v);
                } else {
                    h.copySecondHalfBlock(j, i);
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
                    h.firstHalfInitBlock(i);
                    h.write(i, mod, first, v);
                } else {
                    h.secondHalfInitBlock(i);
                    h.makeChain(k, i);
                    h.write(first ? k : i, mod, false, v);
                }
            } else {            // already written
                h.write(first ? k : i, mod, false, v);
            }
        }

        return h.flag;
    }


    // r == 1
    template<typename T>
    size_t writtenSize(T* A, size_t n, bool flag = false) {
        if (flag) return n;
        defines::ArrayHelper<T> h(A, n);
        return h.lastP().b * defines::blockSize<T>() + (n - h.blocksEnd());
    }


    template<typename T>
    class Holder {
        T* A;
        bool flag;
        const bool malloced;
    public:
        const size_t n;
        Holder(T* A, size_t n,               bool flag = true) : A(A), n(n), flag(flag), malloced(false) { }
        Holder(T* A, size_t n, const T& def, bool flag = true) : A(A), n(n), flag(flag), malloced(false) { fill(def); }
        Holder(size_t n)               : A(new T[n]), n(n), flag(true), malloced(true) { }
        Holder(size_t n, const T& def) : A(new T[n]), n(n), flag(true), malloced(true) { fill(def); }
        ~Holder() { if (malloced) delete[] A; }
        void fill(const T& v) { flag = FillArray::fill(A, n, v); }
        T read(size_t i) const { return FillArray::read(A, n, i, flag); }
        void write(size_t i, const T& v) { flag = FillArray::write(A, n, i, v, flag); }
        size_t writtenSize() { return FillArray::writtenSize(A, n, flag); }
        void operator=(const T& v) { fill(v); }

        struct Proxy {
            Holder<T>& self;
            size_t i;
            Proxy(Holder<T>& self, size_t i) : self(self), i(i) {}
            operator T() const { return self.read(i); }
            Proxy& operator=(const T& v) { self.write(i, v); return *this; }
        };
        struct ConstProxy {
            const Holder<T>& self;
            size_t i;
            ConstProxy(const Holder<T>& self, size_t i) : self(self), i(i) {}
            operator T() const { return self.read(i); }
        };
        ConstProxy operator[](size_t i) const { return ConstProxy(*this, i); }
        Proxy operator[](size_t i) { return Proxy(*this, i); }
    };
}


#endif //O1_FILL_ARRAY_1BIT_H
