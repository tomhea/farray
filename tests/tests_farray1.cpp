#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "../include/farray1.hpp"
#include "test_classes.hpp"

using namespace std;
using namespace std::chrono;
using namespace Farray1Direct;


enum TestType {STRESS, ITERATOR_STRESS};


template<typename T, typename ptr_size1, typename ptr_size2>
bool eq(T* arr, const Farray1<T,ptr_size1>& farr1, const Farray1<T,ptr_size2>& farr2, T* A, int n, bool flag) {
    for (size_t i = 0; i < n; i++) {
        if (!(arr[i] == read(A, n, i, flag) && arr[i] == farr1.read(i) && arr[i] == farr2[i])) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << read(A, n, i, flag) << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool stressTest(int n, int inits, int reads, int writes) {
    typedef int ptr_size1;
    typedef size_t ptr_size2;

    T def = rnd();
    auto farr1 = Farray1<T,ptr_size1>(n, def);
    auto farr2 = Farray1<T,ptr_size2>(n, def);
    T* A = new T[n];
    bool flag = fill(A, n, def);

    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    auto arr = new T[n];
    for (int u = 0; u < n; u++) arr[u] = def;

    if (!eq<T>(arr, farr1, farr2, A, n, flag)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

    int count = 0, lastF = 0;
    for (auto op : actions) {
        count++;
        int i = rand() % n; T v = rnd();
        if (op == 'F') {
            lastF = count;
            if (rand()&1) def = v;
            for (int u = 0; u < n; u++) arr[u] = def;
            flag = fill(A,n,def);
            farr1.fill(def);
            farr2 = def;
        } else if (op == 'W') {
            arr[i] = v;
            flag = write(A,n,i,v,flag);
            farr1.write(i, v);
            farr2[i] = v;
        } else {
            if (!(arr[i] == read(A,n,i,flag) && arr[i] == farr1.read(i) && arr[i] == farr2[i])) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!eq<T,ptr_size1,ptr_size2>(arr, farr1, farr2, A, n, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int)flag << ".    op count = " << count << ", lastF = " << lastF << "." << endl;
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


template<typename T, typename ptr_size = size_t>
bool arraySatisfyPred(Farray1<T,ptr_size>& A, const vector<int>& written_indices) {
    vector<bool> isWritten(A.n, false);
    vector<bool> reallyWritten(A.n, false);
    int bsize = defines::blockSize<T,ptr_size>();

    for (int j = defines::ArrayHelper<T,ptr_size>::blocksEnd(A.n); j < A.n; j++) {
        isWritten[j] = true;
    }
    for (auto i : written_indices) {
        if (i >= defines::ArrayHelper<T,ptr_size>::blocksEnd(A.n)) {
            continue;
        }
        for (int j = (i/bsize)*bsize; j < (i/bsize+1)*bsize; j++) {
            isWritten[j] = true;
        }
    }

    for (size_t i : A) reallyWritten[i] = true;

    for (int i = 0; i < A.n; i++) {
        if (isWritten[i] != reallyWritten[i]) {
            cout << "isWritten[" << i << "] = " << isWritten[i] << ", but reallyWritten[" << i << "] = " << reallyWritten[i] << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool iteratorStressTest(int n, int inits, int reads, int writes) {
    typedef size_t ptr_size;

    vector<int> written_indices;
    T def = rnd();
    auto farr = Farray1<T,ptr_size>(n, def);

    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    if (!arraySatisfyPred<T,ptr_size>(farr, written_indices)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

    int count = 0, lastF = 0;
    for (auto op : actions) {
        count++;
        int i = rand() % n; T v = rnd();
        if (op == 'F') {
            lastF = count;
            if (rand()&1) def = v;
            farr = def;
            written_indices.clear();
        } else if (op == 'W') {
            farr[i] = v;
            written_indices.push_back(i);
        } else {
            T temp = farr[i];
        }

        if (!arraySatisfyPred<T,ptr_size>(farr, written_indices)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ".    op count = " << count << ", lastF = " << lastF << "." << endl;
            return false;
        }
    }

    return true;
}


template<typename T, getRandom<T> rnd>
bool doTest(TestType type, int n, int inits, int reads, int writes) {
    switch(type) {
        case STRESS:
            return stressTest<T, rnd>(n, inits, reads, writes);
        case ITERATOR_STRESS:
            return iteratorStressTest<T, rnd>(n, inits, reads, writes);
        default:
            return false;
    }
}


bool tests(TestType type, vector<size_t> sizes) {
    srand(time(0));
    auto startTime = high_resolution_clock::now();
    int goodTestsNum = 0, testsNum = 0;
    for (auto size : sizes) {
        int i = 100 * max(1 / sqrt(size), 1);
        int r = 3 * i;
        int w = 5 * i;
        cout << "ARRAY SIZE " << size << ":" << endl;
        handleTestResult("X:      ", doTest<X, X::getRandom>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Y:      ", doTest<Y, Y::getRandom>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Z:      ", doTest<Z, Z::getRandom>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("ZZ:     ", doTest<ZZ, ZZ::getRandom>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int:    ", doTest<int, getRand<int, 10000000>>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int64:  ", doTest<uint64_t, getRand<uint64_t, 100000000000>>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int16:  ", doTest<uint16_t, getRand<uint16_t, 60000>>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int8:   ", doTest<uint8_t, getRand<uint8_t, 250>>(type, size, i, r, w), goodTestsNum, testsNum);
        handleTestResult("bool:   ", doTest<bool, getRand<bool, 2>>(type, size, i, r, w), goodTestsNum, testsNum);
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
    bool stressSuc     = tests(STRESS, {1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000});
    bool iterStressSuc = tests(ITERATOR_STRESS, {1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000});

    if (stressSuc && iterStressSuc)
        cout << "All Success!" << endl;
    else
        cout << "stress: " << stressSuc << ",  iter: " << iterStressSuc << "." << endl;

    return 0;
}
