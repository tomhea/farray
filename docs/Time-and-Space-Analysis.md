# Time and Space Analysis

## Timings Results and Statistics

The [timings](https://github.com/tomhea/farray/tree/master/timings) folder contains code for timing the project's classes.<br />
The implemented arrays are faster than a normal array, when calling `fill(v)` at least ~0.05% of the times.<br>
See the last full results of [Farray1](https://github.com/tomhea/farray/blob/master/timings/times_farray1_output.txt), [Farray](), and [NFarray]().

## Space Analysis
The `Farray1Direct` functions only need the uses 1 extra bit - the functions request a boolean flag beside the array and its size.

The `Farray1` class is simpler to use but holds a pointer to the array, its (constant) size, the flag, and a constant boolean indicating if the array was allocated by the Farray constructor. So it's debatable if it is pure 1 extra bit, or a bit more _(pun intended)_.

The `Farray` takes an extra word and the default value saved separately, but it is a bit faster than Farray1 because the block size is smaller.

The `NFarray` takes a few extra words of memory than `Farray` but allows you to sum all the values in the array in _O(1)_ time, and to add/multiply all values simultaneously in _O(1)_.

## Time Analysis

As you know - every operation in this project is O(1) time and consumes O(1) extra space (1 bit, or a bunch of memory words).

In this part we will show the number of read/write accesses in every operation:<br />
*More information can be found in the code itself.<br />
*If r(read) or w(write) not specified - it's zero.<br />
*HB stands for accessing of the size of a Half Block.<br>
*The numbers were directly calculated from the implementation. For more details read the [paper](https://arxiv.org/abs/1709.08900), and then view the [code](https://github.com/tomhea/farray).

---

### Farray1 / Direct Functions
Half Block size is `(sizeof(ptr_size)*2+sizeof(T)-1) / sizeof(T) + 1`, while `ptr_size` is by-default `size_t`.
```c
// w <= 2HB+1
fill(v);

// r <= 5
read(i);

// already_written: r <=  6, w <=     2    (or if another index in the same block was written)
//     first_write: r <= 13, w <= 5HB+7
write(i, v);

// r == 1
writtenSize();

// r == 1
iterator::operator++();
// r == 3
iterator::operator*();
```
