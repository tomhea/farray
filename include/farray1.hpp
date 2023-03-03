#pragma once


/*
Implementation of "In-Place Initializable Arrays" paper - https://arxiv.org/abs/1709.08900
 It's an array implementation with the fill(v), read(i), write(i,v) methods - all in O(1) worst time complexity.
 read and write are obvious, and fill sets all the cells at once.
 The special key in this implementation - is that it takes only 1 bit of redundancy, beyond the given array.
 That bit, called flag, will be given to read & write, and will be returned from the fill & write.
  if flag is true  - the array can be viewed as a normal array - it is fully initialized, and can be accessed normally.
  if flag is false - the array isn't completed yet, and must be accessed with the fill/read/write methods.
 In order to prove that this implementation only uses the array and the 1bit as it's playground:
  each call requests the array, its size, and the flag (assumed to be false if unspecified),
  and doesn't maintain any other data structure between calls. not a single bit more.
  It does use the ArrayHelper struct - but it's created statically and disappears after the call is over.
 The Farray1 class is for a simple use of the fill array.
  It does only save the array pointer and the flag as non-constants fields.
changes from the article:
 Bug fixing: the article's "extend" implementation should've used 2b-1 instead of 2b+1.
 A[i].first.p.ptr saves the already i/block_size index (the block index).
 All functions and macros are noted with the upper bound of their read and write accesses (HB is HalfBlock size).
*/


namespace Farray1Direct {
    namespace defines {
        #pragma pack(push, 1)
        template<typename T, typename ptr_size>
        struct ptrBdef { ptr_size ptr; ptr_size b; T def; };

        template<typename T, typename ptr_size, int N>
        union sizeT {
            T v[N];
            ptrBdef<T, ptr_size> p;
        };

        template<typename T, typename ptr_size, int N>
        struct Block {
            sizeT<T,ptr_size,N> first, second;
        };
        #pragma pack(pop)

        template<typename T, typename ptr_size>
        constexpr size_t halfBlockSize() {
            return ((sizeof(ptr_size)*2+sizeof(T)-1)/sizeof(T)+1);
        }
        template<typename T, typename ptr_size>
        constexpr size_t blockSize() {
            return 2*halfBlockSize<T, ptr_size>();
        }


        template<typename T, typename ptr_size>
        struct ArrayHelper {
        public:
            Block<T,ptr_size,halfBlockSize<T,ptr_size>()>* A;
            size_t n;
            bool flag;
            ptr_size b;
            T def;

            // r <= 2
            ArrayHelper(const T* A, size_t n, bool flag = true)
            : A((Block<T,ptr_size,halfBlockSize<T,ptr_size>()>*)A), n(n), flag(flag) {
                if (!flag) {
                    auto p = lastP();
                    b = p.b;
                    def = p.def;
                }
            }
            size_t numBlocks() const { return n / blockSize<T,ptr_size>(); }
            size_t blocksEnd() const { return numBlocks() * blockSize<T,ptr_size>(); }
            static size_t numBlocks(size_t n) { return n / blockSize<T,ptr_size>(); }
            static size_t blocksEnd(size_t n) { return numBlocks(n) * blockSize<T,ptr_size>(); }

            ptrBdef<T,ptr_size>& lastP() { return A[numBlocks()-1].first.p; }
            void expendB() { flag = ((b=++lastP().b) == numBlocks()); }  // r == 1, w == 1
            void fillBottom(const T& v) { auto& p = lastP(); p.b = 0; p.def = v; }   // w == 2

            ptr_size setIndices(size_t i, size_t& mod, bool& first) const {
                size_t hbs = halfBlockSize<T,ptr_size>();
                mod = i % hbs;
                i /= hbs;
                first = (i%2 == 0);
                i /= 2;
                return (ptr_size)i;
            }

            // r == 2
            bool chainedTo(ptr_size i, ptr_size& k) const {
                k = A[i].first.p.ptr;
                return (k != i) && (k < numBlocks()) && ((i<b) ^ (k<b))
                    && (A[k].first.p.ptr == i);
            }
            // w == 2
            void makeChain(ptr_size i, ptr_size& k) {
                A[i].first.p.ptr = k;
                A[k].first.p.ptr = i;
            }

            // r <= 2, w <= 2HB+1
            void initBlock(ptr_size i) {
                 firstHalfInitBlock(i);
                secondHalfInitBlock(i);
            }
            // r <= 2, w <= HB+1
            void firstHalfInitBlock(ptr_size i) {
                for (int t = 0; t < halfBlockSize<T,ptr_size>(); t++)
                    A[i].first.v[t] = def;
                breakChain(i);
            }
            // w == HB
            void secondHalfInitBlock(ptr_size i) {
                for (int t = 0; t < halfBlockSize<T,ptr_size>(); t++)
                    A[i].second.v[t] = def;
            }

            // r <= 2, w <= 1
            void breakChain(ptr_size i) {
                ptr_size k;
                if (chainedTo(i, k))
                    A[k].first.p.ptr = k;
            }

            // r <= 5, w <= 2HB+2
            ptr_size extend() {
                ptr_size k;
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
            T read(ptr_size i, size_t mod, bool first) const {
                return first ? A[i].first.v[mod] : A[i].second.v[mod];
            }

            /// lines  6-7  in iwrite (r <= 2, w <= 2)
            void write(ptr_size i, size_t mod, bool first, const T& v) {
                if (first) {
                    A[i].first.v [mod] = v;
                    breakChain(i);
                } else {
                    A[i].second.v[mod] = v;
                }
            }

            // w == HB
            void copySecondHalfBlock(ptr_size to, ptr_size from) { A[to].second = A[from].second; }
        };
    }


    // w <= 2HB+1
    template<typename T, typename ptr_size = size_t>
    bool fill(T* A, size_t n, const T& v) {
        defines::ArrayHelper<T,ptr_size> h(A, n);
        for (size_t i = h.blocksEnd(); i < n; i++) A[i] = v;
        if (h.numBlocks() == 0) return true;
        h.fillBottom(v);
        return false;
    }


    // r <= 5
    template<typename T, typename ptr_size = size_t>
    T read(const T* A, size_t n, size_t index, bool flag = false) {
        defines::ArrayHelper<T,ptr_size> h(A, n, flag);
        index %= n;

        if (flag || index >= h.blocksEnd()) {      // arr functions as regular array
            return A[index];
        }

        size_t mod;
        bool first, chained;
        ptr_size i = h.setIndices(index, mod, first), k;
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
    template<typename T, typename ptr_size = size_t>
    bool write(T* A, size_t n, size_t index, const T& v, bool flag = false) {
        defines::ArrayHelper<T,ptr_size> h(A, n, flag);
        index %= n;

        if (flag || index >= h.blocksEnd()) {      // arr functions as regular array
            A[index] = v;
            return flag;
        }

        size_t mod;
        bool first, chained;
        ptr_size i = h.setIndices(index, mod, first), k;
        chained = h.chainedTo(i, k);

        if (i < h.b) {    // UCA
            if ( chained) {     // not written
                ptr_size j = h.extend();
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
    template<typename T, typename ptr_size = size_t>
    size_t writtenSize(T* A, size_t n, bool flag = false) {
        if (flag) return n;
        defines::ArrayHelper<T,ptr_size> h(A, n);
        return h.lastP().b * defines::blockSize<T,ptr_size>() + (n - h.blocksEnd());
    }


    template<typename T, typename ptr_size>
    class iterator {
        T* A;
        size_t n;
        bool flag;

        size_t i;
    public:

        iterator(T* A, size_t n, bool start, bool flag) : A(A), n(n), flag(flag), i(start ? 0 : n) {
            size_t afterLastBlock = defines::ArrayHelper<T,ptr_size>::blocksEnd(n);
            size_t belowB = writtenSize<T,ptr_size>(A, n, flag) - (n - afterLastBlock);
            if (i < n && i == belowB) i = afterLastBlock;
        }
        bool operator==(class iterator& o) { return i == o.i; }
        bool operator!=(class iterator& o) { return !(*this == o); }
        // r == 3
        size_t operator*() {
            defines::ArrayHelper<T,ptr_size> ah(A, n, flag);
            size_t bs = defines::blockSize<T,ptr_size>();
            size_t mod=i%bs;
            ptr_size bi = i/bs, k;

            size_t index = i;
            if (!flag && ah.chainedTo(bi, k))
                index = k  * bs + mod;
            return index;
        }
        // r == 1
        iterator& operator++() {
            size_t afterLastBlock = defines::ArrayHelper<T, ptr_size>::blocksEnd(n);
            size_t belowB = writtenSize<T, ptr_size>(A, n, flag) - (n - afterLastBlock);
            i++;
            if (i == belowB)
                i = afterLastBlock;
            return *this;
        }
    };

    /// Iterates over indices of written values.
    template<typename T, typename ptr_size = size_t>
    iterator<T,ptr_size> begin(T* A, size_t n, bool flag = false) { return iterator<T,ptr_size>(A, n, true,  flag); }
    template<typename T, typename ptr_size = size_t>
    iterator<T,ptr_size>   end(T* A, size_t n, bool flag = false) { return iterator<T,ptr_size>(A, n, false, flag); }
}


template<typename T, typename ptr_size = size_t>
class Farray1 {
    T* A;
    bool flag;
    const bool malloced;
public:
    const size_t n;
    Farray1(T* A, size_t n, const T& def, bool flag = true) : A(A), n(n), flag(flag), malloced(false) { fill(def); }

#ifndef FARRAY1_NO_DYNAMIC_ALLOCATIONS
    Farray1(size_t n, const T& def) : A(new T[n]), n(n), flag(true), malloced(true) { fill(def); }
    ~Farray1() { if (malloced) delete[] A; }
#endif

    void fill(const T& v) { flag = Farray1Direct::fill<T,ptr_size>(A, n, v); }
    T read(size_t i) const { return Farray1Direct::read<T,ptr_size>(A, n, i, flag); }
    void write(size_t i, const T& v) { flag = Farray1Direct::write<T,ptr_size>(A, n, i, v, flag); }
    size_t writtenSize() { return Farray1Direct::writtenSize<T,ptr_size>(A, n, flag); }
    void operator=(const T& v) { fill(v); }

    struct Proxy {
        Farray1<T,ptr_size>& self;
        size_t i;
        Proxy(Farray1<T,ptr_size>& self, size_t i) : self(self), i(i) {}
        operator T() const { return self.read(i); }
        Proxy& operator=(const T& v) { self.write(i, v); return *this; }
        Proxy& operator=(const Proxy& p) { self.write(i, (T)p); return *this; }
    };
    struct ConstProxy {
        const Farray1<T,ptr_size>& self;
        size_t i;
        ConstProxy(const Farray1<T,ptr_size>& self, size_t i) : self(self), i(i) {}
        operator T() const { return self.read(i); }
    };
    ConstProxy operator[](size_t i) const { return ConstProxy(*this, i); }
    Proxy operator[](size_t i) { return Proxy(*this, i); }

    /// Iterates over indices of written values.
    Farray1Direct::iterator<T,ptr_size> begin() { return Farray1Direct::begin<T,ptr_size>(A, n, flag); }
    Farray1Direct::iterator<T,ptr_size>   end() { return Farray1Direct::  end<T,ptr_size>(A, n, flag); }
};
