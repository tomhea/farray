# Farray - Fillable Array
C++ Header-only Implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper.

Templated array with constant-time fill(v), read(i), write(i,v) operations, all with 1 bit of extra memory.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words.

I wrote a **[Medium article](https://link.medium.com/Q8YbkDJX2bb)** about array initialization and this project. Read it and come back 🧑‍💻. 

# Basic Use:
To use the array, just include the header file. *Thats it.*
```c
#include "farray1.hpp"
```

### Using the Farray1 class:
```c
// initialization (all to 1s)
Farray1<int> A(n, 1);   // Farray1 allocated an array be itself. 
                        // It can also take an already allocated array.

// read & write
A[3] = 5;
int x = A[12] + A[19] + A[3];   // reading (1+1+5)

cout << "This must be seven: " << x << endl;

// simple initialization - all values are now 2020
A = 2020;     

for (int i = 5; i <= 10; i++)
    A[i] = i*i;
    
for (int i = 3; i <= 12; i++)
    cout << A[i] << " ";
```

The output will be:
```
This must be seven: 7
2020 2020 25 36 49 64 81 100 2020 2020 
```

You can also use the `A.fill(v), A.read(i), A.write(i,v)` syntax, instread of `A=v, A[i], A[i]=v`.

# Wiki!

This project has a [wiki page](https://github.com/tomhea/farray/wiki)! containing more information:<br>
* [Short Despription about the algorithm](https://github.com/tomhea/farray/wiki/Short-Description)
* [Advanced Features](https://github.com/tomhea/farray/wiki/Advanced-Features) - iterator, direct-functions, smaller-blocks, templates
* Much More!

