#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

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


// TODO rename functions and commit-push!


template<typename T, typename ptr_size1, typename ptr_size2>
bool eq(T *arr, const Farray1<T, ptr_size1> &farr1, const Farray1<T, ptr_size2> &farr2, T *A, int n, bool flag) {
    for (size_t i = 0; i < n; i++) {
        if (!(arr[i] == read(A, n, i, flag) && arr[i] == farr1.read(i) && arr[i] == farr2[i])) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << read(A, n, i, flag) << "."
                 << endl;
            return false;
        }
    }
    return true;
}


int max(int x, int y) {
    return x > y ? x : y;
}


size_t get_basic_number_of_operations(size_t array_size) {
    return max(10000 / sqrt(array_size), 100);
}


template<typename T, getRandom<T> rnd>
bool stressTest(size_t array_size) {
    const size_t basic_number_of_operations = get_basic_number_of_operations(array_size);
    const size_t init_operations = 1 * basic_number_of_operations;
    size_t read_operations = 3 * basic_number_of_operations;
    size_t write_operations = 5 * basic_number_of_operations;

    typedef int ptr_size1;
    typedef size_t ptr_size2;

    T def = rnd();
    auto farr1 = Farray1<T, ptr_size1>(array_size, def);
    auto farr2 = Farray1<T, ptr_size2>(array_size, def);
    T *A = new T[array_size];
    bool flag = fill(A, array_size, def);

    vector<char> actions;
    actions.reserve(read_operations + write_operations + init_operations);
    for (int i = 0; i < read_operations; i++) actions.emplace_back('R');
    for (int i = 0; i < write_operations; i++) actions.emplace_back('W');
    for (int i = 0; i < init_operations; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    auto arr = new T[array_size];
    for (int u = 0; u < array_size; u++) arr[u] = def;

    if (!eq<T>(arr, farr1, farr2, A, array_size, flag)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

    int count = 0, lastF = 0;
    for (auto op: actions) {
        count++;
        int i = rand() % array_size;
        T v = rnd();
        if (op == 'F') {
            lastF = count;
            if (rand() & 1) def = v;
            for (int u = 0; u < array_size; u++) arr[u] = def;
            flag = fill(A, array_size, def);
            farr1.fill(def);
            farr2 = def;
        } else if (op == 'W') {
            arr[i] = v;
            flag = write(A, array_size, i, v, flag);
            farr1.write(i, v);
            farr2[i] = v;
        } else {
            if (!(arr[i] == read(A, array_size, i, flag) && arr[i] == farr1.read(i) && arr[i] == farr2[i])) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!eq<T, ptr_size1, ptr_size2>(arr, farr1, farr2, A, array_size, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int) flag << ".    op count = " << count << ", lastF = "
                 << lastF << "." << endl;
            return false;
        }
    }

    return true;
}


template<typename T, typename ptr_size = size_t>
bool arraySatisfyPred(Farray1<T, ptr_size> &A, const vector<int> &written_indices) {
    vector<bool> isWritten(A.n, false);
    vector<bool> reallyWritten(A.n, false);
    int bsize = defines::blockSize<T, ptr_size>();

    for (int j = defines::ArrayHelper<T, ptr_size>::blocksEnd(A.n); j < A.n; j++) {
        isWritten[j] = true;
    }
    for (auto i: written_indices) {
        if (i >= defines::ArrayHelper<T, ptr_size>::blocksEnd(A.n)) {
            continue;
        }
        for (int j = (i / bsize) * bsize; j < (i / bsize + 1) * bsize; j++) {
            isWritten[j] = true;
        }
    }

    for (size_t i: A) reallyWritten[i] = true;

    for (int i = 0; i < A.n; i++) {
        if (isWritten[i] != reallyWritten[i]) {
            cout << "isWritten[" << i << "] = " << isWritten[i] << ", but reallyWritten[" << i << "] = "
                 << reallyWritten[i] << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool iteratorStressTest(int array_size) {
    const size_t basic_number_of_operations = get_basic_number_of_operations(array_size);
    const size_t init_operations = 1 * basic_number_of_operations;
    size_t read_operations = 3 * basic_number_of_operations;
    size_t write_operations = 5 * basic_number_of_operations;

    typedef size_t ptr_size;

    vector<int> written_indices;
    T def = rnd();
    auto farr = Farray1<T, ptr_size>(array_size, def);

    vector<char> actions;
    actions.reserve(read_operations + write_operations + init_operations);
    for (int i = 0; i < read_operations; i++) actions.emplace_back('R');
    for (int i = 0; i < write_operations; i++) actions.emplace_back('W');
    for (int i = 0; i < init_operations; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    if (!arraySatisfyPred<T, ptr_size>(farr, written_indices)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

    int count = 0, lastF = 0;
    for (auto op: actions) {
        count++;
        int i = rand() % array_size;
        T v = rnd();
        if (op == 'F') {
            lastF = count;
            if (rand() & 1) def = v;
            farr = def;
            written_indices.clear();
        } else if (op == 'W') {
            farr[i] = v;
            written_indices.push_back(i);
        } else {
            T temp = farr[i];
        }

        if (!arraySatisfyPred<T, ptr_size>(farr, written_indices)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ".    op count = " << count << ", lastF = " << lastF << "." << endl;
            return false;
        }
    }

    return true;
}


TEMPLATE_TEST_CASE_SIG("Test Farray1 with random operations; variable array size", "[stress]",
                       ((size_t array_size), array_size),
                       1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000) {

    REQUIRE(stressTest<X, X::getRandom>(array_size));
    REQUIRE(stressTest<Y, Y::getRandom>(array_size));
    REQUIRE(stressTest<Z, Z::getRandom>(array_size));
    REQUIRE(stressTest<ZZ, ZZ::getRandom>(array_size));

    REQUIRE(stressTest<bool, getRand<bool>>(array_size));
    REQUIRE(stressTest<uint8_t, getRand<uint8_t>>(array_size));
    REQUIRE(stressTest<uint16_t, getRand<uint16_t>>(array_size));
    REQUIRE(stressTest<uint32_t, getRand<uint32_t>>(array_size));
    REQUIRE(stressTest<uint64_t, getRand<uint64_t>>(array_size));
}


TEMPLATE_TEST_CASE_SIG("Test iterating over Farray1, after random operations; variable array size", "[stress]",
                       ((size_t array_size), array_size),
                       1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000) {

    REQUIRE(iteratorStressTest<X, X::getRandom>(array_size));
    REQUIRE(iteratorStressTest<Y, Y::getRandom>(array_size));
    REQUIRE(iteratorStressTest<Z, Z::getRandom>(array_size));
    REQUIRE(iteratorStressTest<ZZ, ZZ::getRandom>(array_size));

    REQUIRE(iteratorStressTest<bool, getRand<bool>>(array_size));
    REQUIRE(iteratorStressTest<uint8_t, getRand<uint8_t>>(array_size));
    REQUIRE(iteratorStressTest<uint16_t, getRand<uint16_t>>(array_size));
    REQUIRE(iteratorStressTest<uint32_t, getRand<uint32_t>>(array_size));
    REQUIRE(iteratorStressTest<uint64_t, getRand<uint64_t>>(array_size));
}

