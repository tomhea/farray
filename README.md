# Farray - Fillable Array
C++ Header-only Implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper.

Templated array with constant-time fill(v), read(i), write(i,v) operations, all with 1 bit of extra memory.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words.<br />
Read it and come back 🧑‍💻. 

# Basic Use:
To use the array, just include the header file. *Thats it.*
```
#include "farray1.hpp"
```

### Using the Farray1 class:
(The simpler option)
```
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

It will output:
```
This must be seven: 7
2020 2020 25 36 49 64 81 100 2020 2020 
```

You can also use the `A.fill(v), A.read(i), A.write(i,v)` syntax, instread of `A=v, A[i], A[i]=v`.

# Short Description:
It is needed for the *Advanced Features* section.

The algorithm uses the lower part of the array (adresses 0-\[almost n\]) as blocks of adjacent cells.<br />
The last cells in the array, which can't form a block, are initialized and accessed normally.<br />
The 1 extra bit, called the flag, is set to 1 if the whole lower part of the array is written. Otherwise it's 0.

If the flag is 1, then the array is fully written, and can be accessed as a regular array.

Otherwise:<br />
A block consists of two half-blocks, each is a union of `|ptr|b|def|` and `value[]`.<br />
The lower part of the array is divided into 2 parts - UCA (lower) and WCA (upper).<br />
The UCA consists of the blocks 0-\[b-1\], while WCA consists of the blocks \[b-last_block\].<br />
b, and the curret default value of the array, are saved in the |...|b|def| part of the last block.

Initialization is done block-wise, i.e. a whole block is initialized at once.<br />
Two blocks with indices (b1,b2) are considered *chained* if b1/b2 are in both UCA/WCA and <br />
`blocks[b1].firstHalf.ptr == b2 && blocks[b2].firstHalf.ptr == b1`.<br />
UCA-block is considered initialized if it is not chained,<br />
 and the block's data is saved it the two half-blocks of the said block.<br />
WCA-block is considered initialized if it is chained,<br />
 and the block's data is saved in the second-half-block of both the said block, and its chained block.<br />
In that sense - every 2 chained blocks have exactly 1 written block, and that's the magic of it all.

When reading, the default value is returned for an uninitialized block, or the cell's data<br />
 (found in the same block, or the one chained to it).<br />
When writing - we might increment b by one, in order to use an extra block (to chain it somehow).


# Advanced Features:

### The iterator:
You can also iterate exactly over the written indices (*O(written)* time).<br />
\*Note that the algorithm writes in blocks, so it will iterate over nearby indices too (but it's ok - the whole array is initialized).<br />
\*The iterator returns a size_t index (can be downcasted to int easily, `for (int i : A)`).
\*The iteration order is not in any particular order.

```
// total number of written indices
cout << "Iterating over " << A.writtenSize() << "indices:" << endl;

Farray1<int> A(20); 
A = 7;

// prints only the upper part of the array (the only initialized ones)
for (auto i : A) 
	cout << "A[" << i << "] = " << A[i] << endl;
cout << endl << endl;

A[14] = 6; 
A[3] = 13;

// now also prints the 6 and 13 blocks too
for (auto i : A) 
	cout << "A[" << i << "] = " << A[i] << endl;
```

If the block size is six, then the output will be (last 2, and last 14):

```
A[18] = 7
A[19] = 7


A[0] = 7
A[1] = 7
...
A[3] = 13
...
A[5] = 7
A[12] = 7
...
A[14] = 6
...
A[19] = 7
```

### Using smaller blocks:

The block size is `2 * ((sizeof(ptr_size)*2+sizeof(T)-1)/sizeof(T)+1)`, with the default `ptr_size` is `size_t`.<br />
The block size for a 4-byte int and a 8-byte size_t is 10 ints (40 bytes), so first writes are taking quite a lot of memory accesses,<br />
and the iterator and the fill operation are affected too.

The Farray1 class can get a second template argument - which is the `ptr_size`.<br />
It can be as small as you want, but it will work (for an n-bit unsigned ptr_size) with #blocks < 2<sup>n</sup> arrays.<br />
For example, an uint16_t (16-bit) ptr_size can be used with a char (1-byte) array of up to 2<sup>16</sup> blocks, or 2<sup>16</sup>\*5 bytes.<br />
```
Farray<char, uint16_t> A(200000);
A = 'T';
A[180010] = 'm';
A[180009] = 'o';
for (int i = 0; i < 3; i++) 
	cout << A[i+180008];
```


### Using the direct functions:

If you can't spare the extra bytes of having an Farray1 instance (and why would you?),
You can use the fill, read, write functions directly, while specifying the Array, its length and the flag each time.

A bit more complicated, but a (~100)bit(s) less memory 😉.

All direct functions receive an allocated array, its length, and the extra bit (as a boolean flag).
```
using namespace Farray1Direct;

// initialization
auto A = new int[n];       // A can also be static array, like int A[N];
bool flag = fill(A, n, 1);    // A is initialized to 1s.

flag = write(A, n, 3, 5, flag);    // writing 5 to index 3
int x = read(A, n, 12, flag) + read(A, n, 19, flag) + read(A, n, 3, flag);    // reading (1+1+5)

cout << "This must be seven: " << x << endl;

flag = fill(A, n, 18);

for (int i = 5; i <= 10; i++)
    flag = write(A, n, i, i*i, flag);
    
for (int i = 3; i <= 12; i++)
    cout << read(A, n, i, flag) << " ";
    
delete[] A;
```
It will output the same as the Farray1 code.

### It is templated!:

You can also use **structs**, **classes**, and **any datatype**, with both Farray1 and the direct functions:

```
struct Student {
    uint8_t age;
    string name;
    uint64_t id;
    double avg_grades;
};
Student studs[500];

auto studs = Farray<Student>(500);
studs = {{21, "noName", 1234, 99.3}};   // init all

Farray1<void*> voidptrs(678, 0);

auto hist = new uint32_t[1000000];
bool hist_f = fill(hist, 1000000, 0);
// write and read stuff...
delete hist[];

Farray1<int16_t> ram4k(2048, 0x9090);
while (true) {
    if (ram4k.writtenSize() > 1700) 
        // do something
    // write and read stuff...
}
```
