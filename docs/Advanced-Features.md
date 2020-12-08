Before reading this part, make sure you read the [Short Description](https://github.com/tomhea/farray/wiki/Short-Description) page.

### The iterator
You can also iterate exactly over the written indices (*O(written)* time).<br />
\*Note that the algorithm writes in blocks, so it will iterate over nearby indices too (but it's ok - the whole array is initialized).<br>
\*The iteration order is not in the indices order (otherwise it would be an O(n) sort).<br>
\*The iterator returns a size_t index.

```c
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

### Using smaller blocks

The block size is `2 * ((sizeof(ptr_size)*2+sizeof(T)-1)/sizeof(T)+1)`, with the default `ptr_size` is `size_t`.<br />
The block size for a 4-byte int and a 8-byte size_t is 10 ints (40 bytes), so first writes are taking quite a lot of memory accesses,<br />
and the iterator and the fill operation are affected too.

The Farray1 class can get a second template argument - which is the `ptr_size`.<br />
It can be as small as you want, but it will work (for an n-bit unsigned ptr_size) with #blocks < 2<sup>n</sup> arrays.<br />
For example, an uint16_t (16-bit) ptr_size can be used with a char (1-byte) array of up to 2<sup>16</sup> blocks, or 2<sup>16</sup>\*5 bytes.

```c
Farray<char, uint16_t> A(200000);
A = 'T';
A[180010] = 'm';
A[180009] = 'o';
for (int i = 0; i < 3; i++) 
	cout << A[i+180008];
```


### Using the direct functions

If you can't spare the extra bytes of having a Farray1 instance (and why would you?),
You can use the fill, read, write functions directly, while specifying the Array, its length, and the flag each time.

A bit more complicated, but a (~100)bit(s) less memory 😉.

All direct functions receive an allocated array, its length, and the extra bit (as a boolean flag).
```c
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

### It is templated!

You can also use **structs**, **classes**, and **any datatype**, with both Farray1 and the direct functions:

```c
struct Student {
    uint8_t age;
    string name;
    uint64_t id;
    double avg_grades;
};
Student studs[500];

Farray<Student> studs(500);
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
