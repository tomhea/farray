#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "../include/farray1.hpp"
#include "../tests/test_classes.hpp"

using namespace std;
using namespace std::chrono;


constexpr bool show_farray1_direct_speedup = false;


/// This program prints the speedup gained from using Farray1 over using a regular array.
/// It iterates over the intensity of fill-operations in the read/write/fill operations (in %).
/// It also iterates on different array sizes.


template<typename T, getRandom<T> rnd>
double O1_timed_vs_regular(int n, int inits, int reads, int writes, double* holderVsPlain=nullptr) {
    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('F');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);
    actions[0] = 'F';

    T* A = new T[n]; bool flag = true; T last;

    auto startTime = high_resolution_clock::now();
    for (auto op : actions) {
        int i = rand() % n; T v = rnd();
        if (op == 'F') flag = Farray1Direct::fill(A,n,v);
        else if (op == 'W') flag = Farray1Direct::write(A,n,i,v,flag);
        else last = Farray1Direct::read(A,n,i,flag);
    }
    auto endTime = high_resolution_clock::now();
    auto ms1 = duration_cast<microseconds>(endTime - startTime).count();

    if (holderVsPlain) {
        auto arr = Farray1<T>(n);
        startTime = high_resolution_clock::now();
        for (auto op : actions) {
            int i = rand() % n; T v = rnd();
            if (op == 'F') arr = v;
            else if (op == 'W') arr[i] = v;
            else last = arr[i];
        }
        endTime = high_resolution_clock::now();
        auto ms2 = duration_cast<microseconds>(endTime - startTime).count();
        *holderVsPlain = ms1/(double)ms2;
    }

    startTime = high_resolution_clock::now();
    for (auto op : actions) {
        int i = rand() % n; T v = rnd();
        if (op == 'F') for (int u = 0; u < n; u++) A[u] = v;
        else if (op == 'W') A[i] = v;
        else last = std::move(A[i]);
    }
    endTime = high_resolution_clock::now();
    auto ms3 = duration_cast<microseconds>(endTime - startTime).count();
    cout << (int)(((int*)&last)[0])*0 << "\b \b" << flush;   // use 'last' so that the compiler won't ignore the read operations.
    return ms3/(double)ms1;
}


void handleTimeResult(const string& pretext, size_t size, double res, double holderVsPlain) {
    cout << pretext << size << "]  Farray1/array speedup:    " << fixed << setprecision(3) << res;
    if (show_farray1_direct_speedup) {
        cout << "          (using Farray1/direct speedup: " << holderVsPlain << ")";
    }
    cout << endl;
}


void times(vector<size_t> sizes, vector<double> percents, size_t fills = 100) {
    srand(time(0));
    auto startTime = high_resolution_clock::now();
    cout << endl << endl << "Speedups of using Farray instead of regular array (Bigger than 1 is better):" << endl << endl;
    double minPerc = *min_element(percents.begin(), percents.end());
    double maxSize = *max_element(sizes.begin(), sizes.end());
    double holderVsPlain;
    for (auto perc : percents) {
        cout << endl << "--------------------------------" << endl << endl;
        cout << "Scenario:  " << perc << "% of the operations are array-fills:" << endl << endl;
        for (auto size : sizes) {
            double factor = pow(perc / minPerc, 0.5) / pow(size / maxSize, 0.3);
            size_t total = fills / (perc/100) * factor;
            size_t f,r,w;
            r = w = total * (1-perc/100) / 2;
            f = total * perc/100;
            handleTimeResult("X     [", size, O1_timed_vs_regular<X,       X::getRandom>     (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("X     [", size, O1_timed_vs_regular<X,       X::getRandom>     (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("Y     [", size, O1_timed_vs_regular<Y,       Y::getRandom>     (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("Z     [", size, O1_timed_vs_regular<Z,       Z::getRandom>     (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("ZZ    [", size, O1_timed_vs_regular<ZZ,      ZZ::getRandom>    (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("int64 [", size, O1_timed_vs_regular<int64_t, getRand<int64_t>> (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("int32 [", size, O1_timed_vs_regular<int32_t, getRand<int32_t>> (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("int16 [", size, O1_timed_vs_regular<int16_t, getRand<int16_t>> (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("int8  [", size, O1_timed_vs_regular<int8_t,  getRand<int8_t>>  (size, f, r, w, &holderVsPlain), holderVsPlain);
            handleTimeResult("bool  [", size, O1_timed_vs_regular<bool,    getRand<bool>>    (size, f, r, w, &holderVsPlain), holderVsPlain);
            cout << endl;
        }
    }
    auto endTime = high_resolution_clock::now();
    auto ms = duration_cast<microseconds>(endTime - startTime).count();
    cout << "Overall time: " << ((double)ms)/1000000 << "s." << endl;
}


int main() {
    times({10000, 30000, 100000, 300000, 1000000},
          {50, 20, 10, 5, 2, 1, 0.5, 0.2, 0.15, 0.1, 0.07, 0.05, 0.04, 0.03, 0.02, 0.01},
          100);
    return 0;
}
