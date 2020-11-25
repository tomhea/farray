# O1Array
Implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper.

Array implementation with constant-time fill(v), read(i), write(i,v), all with 1 bit of extra memory.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words. Read it and come back :)

All functions get an allocated array, its length, and the extra bit (as a boolean flag).


# Basic Use:
```
#include "fill_array_1bit.hpp"

using namespace std;
using namespace FillArray;

int n = 20;
```
We will show a simple program using O1Array.

### Using the direct functions:
```
// initialization
auto A = new int[n];       // A can also be static array, like int A[N];
bool flag = fill(A, n, 1);    // A is initialized to 1s.

flag = write(A, n, 3, 5, flag);                                             // writing 5 to index 3
int x = read(A, n, 12, flag) + read(A, n, 19, flag) + read(A, n, 3, flag);  // reading (1+1+5)

cout << "This must be seven: " << x << endl;

flag = fill(A, n, 18);

for (int i = 5; i <= 10; i++)
    flag = write(A, n, i, i*i, flag);
    
for (int i = 3; i <= 12; i++)
    cout << read(A, n, i, flag) << " ";
    
delete[] A;
```

And the simpler option:
### Using the Holder class:
```
// initialization (all to 1s)
auto h = Holder<int>(n, 1);    // Holder can allocate an array by itself, 
                               // and can also take an already allocated array.

h.write(3, 5);                                  // simple write: writing 5 to index 3.
int x = h.read(12) + h.read(19) + h.read(3);    // simple read (1+1+5)

cout << "This must be seven: " << x << endl;

h.fill(2020);

for (int i = 5; i <= 10; i++)
    h.write(i, i*i);
    
for (int i = 3; i <= 12; i++)
    cout << h.read(i) << " ";
```

Both will output:
```
This must be seven: 7
2020 2020 25 36 49 64 81 100 2020 2020 
```
