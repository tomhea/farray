#ifndef O1_TESTCLASSES_HPP
#define O1_TESTCLASSES_HPP

#include <iostream>

using namespace std;

template<typename T>
using getRandom = T(*)();


template<typename T, size_t MAX>
T getRand() {
    return (T)((((uint64_t)rand())<<32 | rand()) % MAX);
}


class X {
public:
    uint8_t a,b,c;

    X() = default;
    X(uint8_t a, uint8_t b, uint8_t c) : a(a), b(b), c(c) {}
    X(const X& v) : a(v.a), b(v.b), c(v.c) {};

    bool operator==(const X& v) const { return (a == v.a) && (b == v.b) && (c == v.c); }
    bool operator!=(const X& v) const { return !(*this == v); }

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
    uint x;
    uint8_t y;

    Y() = default;
    Y(uint x, uint8_t y) : x(x), y(y) {}

    bool operator==(const Y& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Y& v) const { return !(*this == v); }

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

    bool operator==(const Z& v) const { return a == v.a && b == v.b; }
    bool operator!=(const Z& v) const { return !(*this == v); }

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

    bool operator==(const ZZ& v) const { return a == v.a && b == v.b && c == v.c && d == v.d && e == v.e; }
    bool operator!=(const ZZ& v) const { return !(*this == v); }

    static ZZ getRandom() {
        return {((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand(), ((uint64_t)rand())<<32 | rand()};
    }

    friend ostream &operator<<( ostream &output, const ZZ& v ) {
        output << "(" << (uint64_t)v.a << ", " << (uint64_t)v.b << ", " << (uint64_t)v.c << ", " << (uint64_t)v.d << ", " << (uint64_t)v.e << ")";
        return output;
    }
};


#endif //O1_TESTCLASSES_HPP
