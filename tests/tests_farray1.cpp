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


/// run with  "-d yes"  to see times and progress
/// run with  "--rng-seed time"  to make the tests randomized


template<typename T, typename ptr_size1, typename ptr_size2>
bool verify_all_four_arrays_equal(T *regular_array, const Farray1<T, ptr_size1> &farray1_using_functions,
                                  const Farray1<T, ptr_size2> &farray2_using_operators, T *farray3_using_Farray1Direct,
                                  int farray3_n, bool farray3_flag) {
    for (size_t i = 0; i < farray3_n; i++) {
        if (!(regular_array[i] == Farray1Direct::read(farray3_using_Farray1Direct, farray3_n, i, farray3_flag) &&
              regular_array[i] == farray1_using_functions.read(i) && regular_array[i] == farray2_using_operators[i])) {
            cout << "index " << i << ":  regular_array[i]=" << regular_array[i]
                 << ", while farray3_using_Farray1Direct.read(i)="
                 << Farray1Direct::read(farray3_using_Farray1Direct, farray3_n, i, farray3_flag) << "."
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


/// Tests reading, writing, and filling a farray (many of these operations, in a random order).
/// After executing these operations, the farray's content will be compared to that of a regular array, that went through the same operations.
///
/// @note: this functions tests Farray1 class (tests both functions and operators) and the Farray1Direct-namespace functions.
///
/// \tparam T array of that type
/// \tparam rnd function that returns a random T
///
/// \param array_size the size of the tested farray
///
/// \return true if test succeed
template<typename T, getRandom<T> rnd>
bool stress_test(size_t array_size) {
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
    bool flag = Farray1Direct::fill(A, array_size, def);

    vector<char> actions;
    actions.reserve(read_operations + write_operations + init_operations);
    for (int i = 0; i < read_operations; i++) actions.emplace_back('R');
    for (int i = 0; i < write_operations; i++) actions.emplace_back('W');
    for (int i = 0; i < init_operations; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    auto arr = new T[array_size];
    for (int u = 0; u < array_size; u++) arr[u] = def;

    if (!verify_all_four_arrays_equal<T>(arr, farr1, farr2, A, array_size, flag)) {
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
            flag = Farray1Direct::fill(A, array_size, def);
            farr1.fill(def);
            farr2 = def;
        } else if (op == 'W') {
            arr[i] = v;
            flag = Farray1Direct::write(A, array_size, i, v, flag);
            farr1.write(i, v);
            farr2[i] = v;
        } else {
            if (!(arr[i] == Farray1Direct::read(A, array_size, i, flag) && arr[i] == farr1.read(i) && arr[i] == farr2[i])) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!verify_all_four_arrays_equal<T, ptr_size1, ptr_size2>(arr, farr1, farr2, A, array_size, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int) flag << ".    op count = " << count << ", lastF = "
                 << lastF << "." << endl;
            return false;
        }
    }

    return true;
}


template<typename T, typename ptr_size = size_t>
bool verify_farray_iterator_goes_through_the_exact_cells_the_algorithm_initialize(Farray1<T, ptr_size> &farray,
                                                                                  const vector<int> &written_indices) {
    vector<bool> isWritten(farray.n, false);
    vector<bool> reallyWritten(farray.n, false);
    int bsize = Farray1Direct::defines::blockSize<T, ptr_size>();

    for (int j = Farray1Direct::defines::ArrayHelper<T, ptr_size>::blocksEnd(farray.n); j < farray.n; j++) {
        isWritten[j] = true;
    }
    for (auto i: written_indices) {
        if (i >= Farray1Direct::defines::ArrayHelper<T, ptr_size>::blocksEnd(farray.n)) {
            continue;
        }
        for (int j = (i / bsize) * bsize; j < (i / bsize + 1) * bsize; j++) {
            isWritten[j] = true;
        }
    }

    for (size_t i: farray) reallyWritten[i] = true;

    for (int i = 0; i < farray.n; i++) {
        if (isWritten[i] != reallyWritten[i]) {
            cout << "isWritten[" << i << "] = " << isWritten[i] << ", but reallyWritten[" << i << "] = "
                 << reallyWritten[i] << "." << endl;
            return false;
        }
    }
    return true;
}


/// Tests that the Farray1 iterator goes through the exact indices the algorithm requires to initialize.
/// Test it on an empty array, and also after many random reading, writing, and filling operations.
///
/// \tparam T array of that type
/// \tparam rnd function that returns a random T
///
/// \param array_size the size of the tested farray
///
/// \return true if test succeed
template<typename T, getRandom<T> rnd>
bool iterator_indices_test(int array_size) {
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

    if (!verify_farray_iterator_goes_through_the_exact_cells_the_algorithm_initialize<T, ptr_size>
            (farr, written_indices)) {
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

        if (!verify_farray_iterator_goes_through_the_exact_cells_the_algorithm_initialize<T, ptr_size>
                (farr, written_indices)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ".    op count = " << count << ", lastF = " << lastF << "." << endl;
            return false;
        }
    }

    return true;
}


TEMPLATE_TEST_CASE_SIG("Stress Test Farray1 with random operations; variable array size",
                       "[stress]",
                       ((size_t array_size), array_size),
                       1, 2, 3) {

    REQUIRE(stress_test<X, X::getRandom>(array_size));
    REQUIRE(stress_test<Y, Y::getRandom>(array_size));
    REQUIRE(stress_test<Z, Z::getRandom>(array_size));
    REQUIRE(stress_test<ZZ, ZZ::getRandom>(array_size));

    REQUIRE(stress_test<bool, getRand<bool>>(array_size));
    REQUIRE(stress_test<uint8_t, getRand<uint8_t>>(array_size));
    REQUIRE(stress_test<uint16_t, getRand<uint16_t>>(array_size));
    REQUIRE(stress_test<uint32_t, getRand<uint32_t>>(array_size));
    REQUIRE(stress_test<uint64_t, getRand<uint64_t>>(array_size));
}


TEMPLATE_TEST_CASE_SIG("Test Farray1 iteration indices, before and after random operations; variable array size",
                       "[stress]",
                       ((size_t array_size), array_size),
                       1, 2, 3) {

    REQUIRE(iterator_indices_test<X, X::getRandom>(array_size));
    REQUIRE(iterator_indices_test<Y, Y::getRandom>(array_size));
    REQUIRE(iterator_indices_test<Z, Z::getRandom>(array_size));
    REQUIRE(iterator_indices_test<ZZ, ZZ::getRandom>(array_size));

    REQUIRE(iterator_indices_test<bool, getRand<bool>>(array_size));
    REQUIRE(iterator_indices_test<uint8_t, getRand<uint8_t>>(array_size));
    REQUIRE(iterator_indices_test<uint16_t, getRand<uint16_t>>(array_size));
    REQUIRE(iterator_indices_test<uint32_t, getRand<uint32_t>>(array_size));
    REQUIRE(iterator_indices_test<uint64_t, getRand<uint64_t>>(array_size));
}

