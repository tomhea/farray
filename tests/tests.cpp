#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "../include/fill_array_1bit.h"
#include "testClasses.hpp"

using namespace std;
using namespace std::chrono;
using namespace FillArray;


template<typename T>
bool eq(T* arr, const Holder<T>& h, T* A, int n, bool flag) {
    for (int i = 0; i < n; i++) {
        if (!(arr[i] == read(A, n, i, flag) && arr[i] == h.read(i))) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << read(A, n, i, flag) << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool stressTest(int n, T def, int inits, int reads, int writes) {
    auto h = Holder<T>(n, def);
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
    if (!eq<T>(arr, h, A, n, flag)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

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
            h.fill(def);
        } else if (op == 'W') {
            arr[i] = v;
            flag = write(A,n,i,v,flag);
            h.write(i, v);
        } else {
            if (!(arr[i] == read(A,n,i,flag) && arr[i] == h.read(i))) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!eq<T>(arr, h, A, n, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int)flag << ".    op count = " << count << ", last2 = " << last2 << "." << endl;
            return false;
        }
    }

    return true;
}


void handleTestResult(const string& pretext, bool successRes, int& goodTestsNum, int& testsNum) {
    testsNum++;
    goodTestsNum += successRes;
    cout << pretext << (successRes ? "Success!" : "Failed.") << endl;
}


int max(int x, int y) {
    return x>y ? x : y;
}


bool tests(vector<size_t> sizes) {
    srand(time(0));
    auto startTime = high_resolution_clock::now();
    int goodTestsNum = 0, testsNum = 0;
    for (auto size : sizes) {
        int i = max(10000 / sqrt(size), 100);
        int r = max(30000 / sqrt(size), 300);
        int w = max(50000 / sqrt(size), 500);
        cout << "ARRAY SIZE " << size << ":" << endl;
        handleTestResult("X:      ", stressTest<X, X::getRandom>(size, {1, 2, 3}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Y:      ", stressTest<Y, Y::getRandom>(size, {14, 56}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Z:      ", stressTest<Z, Z::getRandom>(size, {123456789098, 9876543212345}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("ZZ:     ", stressTest<ZZ, ZZ::getRandom>(size, {5523456789098, 44876543212345, 0, 1348765432578, 446}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int:    ", stressTest<int, getRand<int, 10000000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("size_t: ", stressTest<size_t, getRand<size_t, 100000000000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int16:  ", stressTest<int16_t, getRand<int16_t, 60000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int8:   ", stressTest<int8_t, getRand<int8_t, 250>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("bool:   ", stressTest<bool, getRand<bool, 2>>(size, true, i, r, w), goodTestsNum, testsNum);
        cout << endl;
    }
    auto endTime = high_resolution_clock::now();
    auto ms = duration_cast<microseconds>(endTime - startTime).count();
    cout << "Overall time: " << ((double)ms)/1000000 << "s." << endl;
    bool success = goodTestsNum == testsNum;
    cout << "Tests passed: " << goodTestsNum << "/" << testsNum << ". " << (success ? "Success!" : "Failed.") << endl << endl;
    return success;
}


int main() {
    tests({1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000});
    return 0;
}
