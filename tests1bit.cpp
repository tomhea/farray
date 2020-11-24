#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iostream>
#include <chrono>

#pragma pack(1)

#include "o1array1bit.h"

using namespace std;
using namespace std::chrono;

template<typename T>
using getRandom = T(*)();


template<typename T>
bool eq(T* arr, T* A, int n, bool flag) {
    for (int i = 0; i < n; i++) {
        if (arr[i] != read(A, n, i, flag)) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << read(A, n, i, flag) << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool stressTest(int n, T def, int inits, int reads, int writes) {
    T* A = new T[n];
    bool flag = fill(A, n, def);
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
            if (rand()&1) def = v;
            for (int u = 0; u < n; u++) arr[u] = def;
            flag = fill(A,n,def);
        } else if (op == 'W') {
            arr[i] = v;
            flag = write(A,n,i,v,flag);
        } else {
            if (arr[i] != read(A,n,i,flag)) {
                cout << "Bad Read: at index " << i << "." << endl;
                return false;
            }
        }

        if (!eq<T>(arr, A, n, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int)flag << ".    op count = " << count << ", last2 = " << last2 << "." << endl;
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
    return (T)((((uint64_t)rand())<<32 | rand()) % MAX);
}


template<typename T, getRandom<T> rnd>
double O1_timed_vs_regular(int n, int inits, int reads, int writes) {
    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('I');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);
    actions[0] = 'I';
    T* A = new T[n]; bool flag = true; T last;

    auto start = high_resolution_clock::now();
    for (auto op : actions) {
        int i = rand() % n; T v = rnd();
        if (op == 'I') flag = fill(A,n,v);
        else if (op == 'W') flag = write(A,n,i,v,flag);
        else last = read(A,n,i,flag);
    }
    auto end = high_resolution_clock::now();
    auto ms1 = duration_cast<microseconds>(end - start).count();

    start = high_resolution_clock::now();
    for (auto op : actions) {
        int i = rand() % n; T v = rnd();
        if (op == 'I') for (int u = 0; u < n; u++) A[u] = v;
        else if (op == 'W') A[i] = v;
        else last = A[i];
    }
    end = high_resolution_clock::now();
    auto ms2 = duration_cast<microseconds>(end - start).count();
    cout << (int)(((int*)&last)[0])*0 << "\b \b";
    return (double)ms1/(double)ms2;
}

int max(int x, int y) { return x>y ? x : y; }
void tests(vector<size_t> sizes) {
    for (auto size : sizes) {
        int i = max(10000 / sqrt(size), 100);
        int r = max(30000 / sqrt(size), 300);
        int w = max(50000 / sqrt(size), 500);
        srand(time(0));
        cout << "STARTING SIZE " << size << ":" << endl;
        cout << "X:      " << (stressTest<X, X::getRandom>(size, {1, 2, 3}, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "Y:      " << (stressTest<Y, Y::getRandom>(size, {14, 56}, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "Z:      " << (stressTest<Z, Z::getRandom>(size, {123456789098, 9876543212345}, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "ZZ:     " << (stressTest<ZZ, ZZ::getRandom>(size, {123456789098, 9876543212345, 0, 1348765432578, 446}, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "int:    " << (stressTest<int, getRand<int, 10000000>>(size, 123, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "size_t: " << (stressTest<size_t, getRand<size_t, 100000000000>>(size, 123, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "int16:  " << (stressTest<int16_t, getRand<int16_t, 60000>>(size, 123, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "int8:   " << (stressTest<int8_t, getRand<int8_t, 250>>(size, 123, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << "bool:   " << (stressTest<bool, getRand<bool, 2>>(size, true, i, r, w) ? "Success!" : "Failed.") << endl;
        cout << endl;
    }
}

void times(vector<size_t> sizes, vector<double> percents, size_t fills = 100) {
    cout << endl << endl << "Speedups [<1 is better]: " << endl << endl;
    double minPerc = *min_element(percents.begin(), percents.end());
    double maxSize = *max_element(sizes.begin(), sizes.end());
    for (auto perc : percents) {
        cout << "--------------------------------" << endl << endl;
        cout << perc << "% fills:" << endl << endl;
        for (auto size : sizes) {
            double factor = pow(perc / minPerc, 0.5) / pow(size / maxSize, 0.3);
            size_t total = fills / (perc/100) * factor;
            size_t f,r,w;
            r = w = total * (1-perc/100) / 2;
            f = total * perc/100;
            srand(time(0));
            cout << "X     [" << size << "]:   " << O1_timed_vs_regular<X,       X::getRandom>                  (size, f, r, w) << endl;
            cout << "Y     [" << size << "]:   " << O1_timed_vs_regular<Y,       Y::getRandom>                  (size, f, r, w) << endl;
            cout << "Z     [" << size << "]:   " << O1_timed_vs_regular<Z,       Z::getRandom>                  (size, f, r, w) << endl;
            cout << "ZZ    [" << size << "]:   " << O1_timed_vs_regular<ZZ,      ZZ::getRandom>                 (size, f, r, w) << endl;
            cout << "int   [" << size << "]:   " << O1_timed_vs_regular<int,     getRand<int,     10000000>>    (size, f, r, w) << endl;
            cout << "size_t[" << size << "]:   " << O1_timed_vs_regular<size_t,  getRand<size_t,  100000000000>>(size, f, r, w) << endl;
            cout << "int16 [" << size << "]:   " << O1_timed_vs_regular<int16_t, getRand<int16_t, 60000>>       (size, f, r, w) << endl;
            cout << "int8  [" << size << "]:   " << O1_timed_vs_regular<int8_t,  getRand<int8_t,  250>>         (size, f, r, w) << endl;
            cout << "bool  [" << size << "]:   " << O1_timed_vs_regular<bool,    getRand<bool,    2>>           (size, f, r, w) << endl;
            cout << endl;
        }
    }
}

int main() {
//    cout << defines::halfBlockSize<int>() << endl;
    tests({1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000});
//    times({10000, 30000, 100000, 300000, 1000000}, {0.01, 0.02, 0.03, 0.04, 0.05, 0.07, 0.1, 0.15, 0.2, 0.5, 1, 2, 5, 10, 20, 50}, 100);

    return 0;
}
