#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <memory>
#include <type_traits>
#include <utility>

#include "../include/farray1.hpp"
#include "test_classes.hpp"

using namespace std;
using namespace std::chrono;


/// run with  "-d yes"  to see times and progress
/// run with  "--rng-seed time"  to make the tests randomized


template<typename T, typename ptr_size1, typename ptr_size2>
bool verify_all_four_arrays_equal(T *regular_array, const Farray1<T, ptr_size1> &farray1_ptr_size1,
                                  const Farray1<T, ptr_size2> &farray2_ptr_size2, T *farray3_using_Farray1Direct,
                                  size_t farray3_n, bool farray3_flag) {
    for (size_t i = 0; i < farray3_n; i++) {
        if (!(regular_array[i] == Farray1Direct::read(farray3_using_Farray1Direct, farray3_n, i, farray3_flag) &&
              regular_array[i] == farray1_ptr_size1[i] && regular_array[i] == farray2_ptr_size2[i] &&
              regular_array[i] == farray1_ptr_size1.read(i) && regular_array[i] == farray2_ptr_size2.read(i))) {
            cout << "index " << i << ":" << endl
                 << "               regular_array[i] = " << regular_array[i] << endl
                 << "     farray1<ptr_size1>.read(i) = " << farray1_ptr_size1.read(i) << endl
                 << "          farray1<ptr_size1>[i] = " << (T) farray1_ptr_size1[i] << endl
                 << "     farray2<ptr_size2>.read(i) = " << farray2_ptr_size2.read(i) << endl
                 << "          farray2<ptr_size2>[i] = " << (T) farray2_ptr_size2[i] << endl
                 << "Farray1Direct::read(farray3, i) = "
                 << Farray1Direct::read(farray3_using_Farray1Direct, farray3_n, i, farray3_flag) << "."
                 << endl << endl;
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
    unique_ptr<T[]> A_owner(new T[array_size]);
    T *A = A_owner.get();
    bool flag = Farray1Direct::fill(A, array_size, def);

    vector<char> actions;
    actions.reserve(read_operations + write_operations + init_operations);
    for (size_t i = 0; i < read_operations; i++) actions.emplace_back('R');
    for (size_t i = 0; i < write_operations; i++) actions.emplace_back('W');
    for (size_t i = 0; i < init_operations; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);

    unique_ptr<T[]> arr_owner(new T[array_size]);
    T *arr = arr_owner.get();
    for (size_t u = 0; u < array_size; u++) arr[u] = def;

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
            for (size_t u = 0; u < array_size; u++) arr[u] = def;
            flag = Farray1Direct::fill(A, array_size, def);
            farr1.fill(def);
            farr2 = def;
        } else if (op == 'W') {
            arr[i] = v;
            flag = Farray1Direct::write(A, array_size, i, v, flag);
            farr1.write(i, v);
            farr2[i] = v;
        } else {
            if (!(arr[i] == Farray1Direct::read(A, array_size, i, flag) && arr[i] == farr1.read(i) &&
                  arr[i] == farr2[i])) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!verify_all_four_arrays_equal<T, ptr_size1, ptr_size2>(arr, farr1, farr2, A, array_size, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", value = " << v << "." << endl;
            cout << "N = " << array_size << ".    block-size: " << Farray1Direct::defines::blockSize<T, size_t>()
                 << endl;
            cout << "Last def = " << def << ", flag = " << (int) flag << ".    op count = " << count << ", lastFill = "
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

    for (size_t j = Farray1Direct::defines::ArrayHelper<T, ptr_size>::blocksEnd(farray.n); j < farray.n; j++) {
        isWritten[j] = true;
    }
    for (auto i: written_indices) {
        if ((size_t)i >= Farray1Direct::defines::ArrayHelper<T, ptr_size>::blocksEnd(farray.n)) {
            continue;
        }
        for (int j = (i / bsize) * bsize; j < (i / bsize + 1) * bsize; j++) {
            isWritten[j] = true;
        }
    }

    for (size_t i: farray) reallyWritten[i] = true;

    for (size_t i = 0; i < farray.n; i++) {
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
    for (size_t i = 0; i < read_operations; i++) actions.emplace_back('R');
    for (size_t i = 0; i < write_operations; i++) actions.emplace_back('W');
    for (size_t i = 0; i < init_operations; i++) actions.emplace_back('F');
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
            (void)temp;
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


TEST_CASE("Farray1 cannot be copied (copying would double-delete the buffer)", "[regression]") {
    REQUIRE_FALSE(std::is_copy_constructible<Farray1<int>>::value);
    REQUIRE_FALSE(std::is_copy_assignable<Farray1<int>>::value);
    REQUIRE(std::is_move_constructible<Farray1<int>>::value);
}


TEST_CASE("moved-from Farray1 releases ownership (no double-free)", "[regression]") {
    Farray1<int> a(50, 7);
    a.write(3, 9);
    Farray1<int> b(std::move(a));
    REQUIRE(b.read(3) == 9);
    REQUIRE(b.read(4) == 7);
    REQUIRE(b.n == 50);
    // 'a' is destroyed at scope exit; it must not delete the buffer 'b' now owns
}


TEST_CASE("size-0 Farray1 is safe to construct, access and iterate", "[regression]") {
    Farray1<int> f(0, 5);
    REQUIRE(f.read(0) == 0);
    f.write(0, 7);              // no-op, must not crash
    REQUIRE(f.writtenSize() == 0);
    size_t iterated = 0;
    for (size_t i : f) { (void)i; iterated++; }
    REQUIRE(iterated == 0);
}


TEST_CASE("read/write on a size-0 array don't divide by zero", "[regression]") {
    int dummy[1] = {42};
    REQUIRE(Farray1Direct::read(dummy, 0, 0) == 0);
    REQUIRE_FALSE(Farray1Direct::write(dummy, 0, 0, 7));
    REQUIRE(dummy[0] == 42);
}


TEST_CASE("writtenSize handles flag=false with fewer elements than one block", "[regression]") {
    // blockSize<uint64_t,size_t>() == 6 > 4, so there are no blocks at all
    uint64_t small[4] = {1, 2, 3, 4};
    REQUIRE(Farray1Direct::writtenSize(small, 4, false) == 4);
}


TEST_CASE("iterator maps tail indices to themselves (tail data is not a chain pointer)", "[regression]") {
    // blockSize<uint64_t,size_t>() == 6; n=13 -> blocks {0,1}, tail {12}.
    // Writing 2 at index 0 and 0 at index 12 used to make the iterator treat the
    // tail value as a chain from block 2 to block 0, yielding index 0 instead of 12.
    Farray1<uint64_t> f(13, 99);
    f.write(0, 2);
    f.write(12, 0);
    vector<size_t> seen;
    for (size_t i : f) seen.push_back(i);
    REQUIRE(count(seen.begin(), seen.end(), (size_t)0) == 1);
    REQUIRE(count(seen.begin(), seen.end(), (size_t)12) == 1);
}


TEST_CASE("explicit iteration with rvalue end() compiles and matches range-for", "[regression]") {
    Farray1<int> f(100, 1);
    f.write(5, 7);
    size_t explicit_count = 0;
    for (auto it = f.begin(); it != f.end(); ++it) explicit_count++;
    size_t range_count = 0;
    for (size_t i : f) { (void)i; range_count++; }
    REQUIRE(explicit_count == range_count);
}


TEMPLATE_TEST_CASE_SIG("Stress Test Farray1 with random operations; variable array size",
                       "[stress]",
                       ((size_t array_size), array_size),
                       1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                       15, 20, 30, 40, 50, 60, 70, 80, 90, 100,
                       200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000) {

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
                       1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                       15, 20, 30, 40, 50, 60, 70, 80, 90, 100,
                       200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000) {

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

