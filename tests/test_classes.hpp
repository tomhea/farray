#ifndef FARRAY_TEST_CLASSES_HPP
#define FARRAY_TEST_CLASSES_HPP

#include <iostream>

using namespace std;

template<typename T>
using getRandom = T(*)();


template<typename T>
T getRand() {
    return ((uint64_t)rand())<<32 | (uint64_t)rand();
}


class X {
public:
    uint8_t a,b,c;

    X() = default;
    X(uint8_t a, uint8_t b, uint8_t c) : a(a), b(b), c(c) {}
    X(const X& v) = default;

    bool operator==(const X& v) const { return (a == v.a) && (b == v.b) && (c == v.c); }
    bool operator!=(const X& v) const { return !(*this == v); }
    X& operator=(const X& v) = default;
//    volatile X& operator=(X&& v) volatile { a = v.a; b = v.b; c = v.c; return *this; };

    static X getRandom() {
        return {(uint8_t)(rand()&0xFF), (uint8_t)(rand()&0xFF), (uint8_t)(rand()&0xFF)};
    }

    friend ostream &operator<<( ostream &output, const X& v ) {
        output << "(" << (int)v.a << ", " << (int)v.b << ", " << (int)v.c << ")";
        return output;
    }
};


class Y {
public:
    uint32_t x;
    uint8_t y;

    Y() = default;
    Y(uint32_t x, uint8_t y) : x(x), y(y) {}
    Y(const Y&) = default;

    bool operator==(const Y& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Y& v) const { return !(*this == v); }
    Y& operator=(const Y& v) = default;
//    volatile Y& operator=(Y&& v) volatile { x = v.x; y = v.y; return *this; };

    static Y getRandom() {
        return {rand()&0xFFFFFFFF, (uint8_t)(rand()&0xFF)};
    }

    friend ostream &operator<<( ostream &output, const Y& v ) {
        output << "(" << (int)v.x << ", " << (int)v.y << ")";
        return output;
    }
};


class Z {
public:
    uint64_t a,b;

    Z() = default;
    Z(uint64_t a, uint64_t b) : a(a), b(b) {}
    Z(const Z& v) = default;

    bool operator==(const Z& v) const { return a == v.a && b == v.b; }
    bool operator!=(const Z& v) const { return !(*this == v); }
    Z& operator=(const Z& v) = default;
//    volatile Z& operator=(Z&& v) volatile { a = v.a; b = v.b; return *this; };

    static Z getRandom() {
        return {((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand()};
    }

    friend ostream &operator<<( ostream &output, const Z& v ) {
        output << "(" << (uint64_t)v.a << ", " << (uint64_t)v.b << ")";
        return output;
    }
};


class ZZ {
public:
    uint64_t a,b,c,d,e;

    ZZ() = default;
    ZZ(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) : a(a), b(b), c(c), d(d), e(e) {}
    ZZ(const ZZ& v) = default;

    bool operator==(const ZZ& v) const { return a == v.a && b == v.b && c == v.c && d == v.d && e == v.e; }
    bool operator!=(const ZZ& v) const { return !(*this == v); }
    ZZ& operator=(const ZZ& v) = default;
//    volatile ZZ& operator=(ZZ&& v) volatile { a = v.a; b = v.b; c = v.c; d = v.d; e = v.e; return *this; };

    static ZZ getRandom() {
        return {((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand()};
    }

    friend ostream &operator<<( ostream &output, const ZZ& v ) {
        output << "(" << (uint64_t)v.a << ", " << (uint64_t)v.b << ", " << (uint64_t)v.c << ", " << (uint64_t)v.d << ", " << (uint64_t)v.e << ")";
        return output;
    }
};


#endif //FARRAY_TEST_CLASSES_HPP
