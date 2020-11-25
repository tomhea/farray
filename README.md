# O1Array
Array implementation with constant-time fill(v), read(i), write(i,v), all with 1 bit of extra memory.

Implementing the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper.

All functions get the allocated array, its length, and the extra bit (as a boolean flag).


# Basic Use:
```
#include "fill_array_1bit.hpp"

using namespace std;
using namespace FillArray;
```
we will show a simple program using O1Array.

### Using the direct functions:
```
// initialization
auto A = new int[n];       // A can also be static array, like int A[N];
bool flag = fill(A, n, 1);    // A is initialized to 1s.

flag = write(A, n, 3, 5, flag);
int x = read(A, n, 12, flag) + read(A, n, 19, flag) + read(A, n, 3, flag);

cout << "This must be zero: " << read(A, n, 7, flag) << endl;

flag = fill(A, n, 18);

for (int i = 5; i <= 10; i++)
    flag = write(A, n, i, i*i, flag);
    
for (int i = 3; i <= 12; i++)
    cout << read(A, n, i, flag) << " ";
    
delete[] A;
```

### Using the Holder class:
```
// initialization (all to 1s)
auto h = Holder<int>(n, 1);    // Holder can allocate an array by itself, 
                               // and can also take an already allocated array.

h.write(3, 5);
int x = h.read(12) + h.read(19) + h.read(3);   // simple read

cout << "This must be five: " << x << endl;

h.fill(2020);

for (int i = 5; i <= 10; i++)
    h.write(i, i*i);
    
for (int i = 3; i <= 12; i++)
    cout << h.read(i) << " ";
```

Both will output:
```
This must be zero: 0
2020 2020 25 36 49 64 81 100 2020 2020 
```
