# O1Array
Header-only Implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper.

(templated) Array with constant-time fill(v), read(i), write(i,v), all with 1 bit of extra memory.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words. Read it and come back 🧑‍💻. 

The project implemented the three functions, and the Holder wrapper.

# Basic Use:
To use the array, just include the header file. *Thats it.*
```
#include "fill_array_1bit.hpp"

using namespace FillArray;
```
We will show a simple program using O1Array.

### Using the Holder class:
(The simpler option)
```
// initialization (all to 1s)
auto h = Holder<int>(n, 1);    // Holder can allocate an array by itself, 
                               // or can take an already allocated array.

h.write(3, 5);    // writing 5 to index 3
int x = h.read(12) + h.read(19) + h.read(3);    // reading (1+1+5)

cout << "This must be seven: " << x << endl;

h.fill(2020);

for (int i = 5; i <= 10; i++)
    h.write(i, i*i);
    
for (int i = 3; i <= 12; i++)
    cout << h.read(i) << " ";
```

It will output:
```
This must be seven: 7
2020 2020 25 36 49 64 81 100 2020 2020 
```

If you can't spare the extra bytes of having an Holder instance (and why would you?),
You can use the fill, read, write functions directly, while specifying the Array, its size and the flag each time.

A bit more complicated, but a (~100)bit(s) less memory 😉.

### Using the direct functions:
All direct functions recieve an allocated array, its length, and the extra bit (as a boolean flag).
```
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
It will output the same as the above code.

You can also use **structs**, **classes**, and **any** datatype:

```
struct Student {
    uint8_t age;
    string name;
    uint64_t id;
    double avg_grades;
};
Student studs[500];

bool flag = fill(studs, 500, {21, "noName", 1234, 99.3});

auto h1 = Holder<Student>(studs, 500, {21, "noName", 1234, 99.3});

// can also be dynamically allocated. For example using Holder:
auto h2 = Holder<Student>(300, {21, "noName", 1234, 99.3});

auto voidptrH = Holder<void*>(678, 0);

auto hist = new uint32_t[1000000];
bool hist_f = fill(hist, 1000000, 0);
// write and read stuff...
delete hist[];

auto ram4k = Holder<int16_t>(2048, 0x9797);
while (true) {
    if (ram4k.writtenSize() > 1700) 
        notify_user();
    // write and read stuff...
}

```
