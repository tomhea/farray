#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>

#include "o1array.h"

using namespace std;



template<typename T>
using getString = string(*)(const T&);
template<typename T>
using getRandom = T(*)();

template<typename T>
bool eq(T* arr, O1Array<T> A, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] != A.read(i)) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << A.read(i) << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool stressTest(int n, T def, int inits, int reads, int writes) {
    T* aaaaa = new T[n];
    auto A = O1Array<T>(aaaaa, n, def);
    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('I');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);
    auto arr = new T[n];
    for (int u = 0; u < n; u++) arr[u] = def;

    int count = 0;
    int last2 = 0;
    for (auto op : actions) {
        count++;
        int i = rand() % n; T v = rnd();
        if (op == 'I') {
            last2 = count;
            if (rand()&1) {
                def = v;
                for (int u = 0; u < n; u++) arr[u] = def;
                A.init(def);
            } else {
                for (int u = 0; u < n; u++) arr[u] = def;
                A.init();
            }
        } else if (op == 'W') {
            arr[i] = v;
            A.write(i, v);
        } else {
            if (arr[i] != A.read(i)) {
                cout << "Bad Read: at index " << i << "." << endl;
                return false;
            }
        }

        if (!eq<T>(arr, A, n)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ".    op count = " << count << ", last2 = " << last2 << "." << endl;
            return false;
        }
    }

    return true;
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
        output << "(" << (uint64_t)v.a << ", " << (uint64_t)v.b << (uint64_t)v.c << ", " << (uint64_t)v.d << ", " << (uint64_t)v.e << ", " << ")";
        return output;
    }
};

template<typename T, size_t MAX>
T getRand() {
    return (T)(rand() % MAX);
}



int main() {
    size_t sizes[] = {1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000};
    for (auto size : sizes) {
        srand(time(0));
        cout << "STARTING SIZE " << size << ":" << endl;
        cout << "X:      " << (stressTest<X, X::getRandom>(size, {1, 2, 3}, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "Y:      " << (stressTest<Y, Y::getRandom>(size, {14, 56}, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "Z:      " << (stressTest<Z, Z::getRandom>(size, {123456789098, 9876543212345}, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "ZZ:     " << (stressTest<ZZ, ZZ::getRandom>(size, {123456789098, 9876543212345, 0, 1348765432578, 446}, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "int:    " << (stressTest<int, getRand<int, 10000000>>(size, 123, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "size_t: " << (stressTest<size_t, getRand<size_t, 100000000000>>(size, 123, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "int16:  " << (stressTest<int16_t, getRand<int16_t, 60000>>(size, 123, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "int8:   " << (stressTest<int8_t, getRand<int8_t, 250>>(size, 123, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << "bool:   " << (stressTest<bool, getRand<bool, 2>>(size, true, 1000, 3000, 5000) ? "Success!" : "Failed.") << endl;
        cout << endl;
    }
    return 0;
}
