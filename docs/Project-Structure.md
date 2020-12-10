# Project Structure

An overview of the project structure.

## `include`
The [include](https://github.com/tomhea/farray/tree/master/include) folder contains the header files.<br/>
They are completely independent and can be used by simply downloading and including them.

### file farray1.hpp:
* `namespace Farray1Direct` - 1bit implementation of fill, read, write, writtenSize, begin/end, <br/>
and an interior `namespace defines` for the interior functions.
* `class Farray1` - The wrapper for the 1bit functions (proxy operator[], operator=, iterator, no need for A,n,flag each call).

### file farray.hpp:
* `class Farray` - The implementation of the log(n) bits (b and def outside), with the Farray1 features ([], =, iterator, ...).

### file nfarray.hpp:
* `class NFarray : public Farray` - extends `Farray` with numerical features:<br/>
++,--,+=,-=,*=,/=, (for proxy, and for the whole Array), and maintaining the sum of all vars. All of the operations are still O(1).

## `docs`
The [docs](https://github.com/tomhea/farray/tree/master/docs) folder contains the project's [GitHub-Pages Site](https://tomhea.github.io/farray/) files.

## `tests`
The [tests](https://github.com/tomhea/farray/tree/master/tests) folder contains random-tests for the different header files.

## `timings`
The [timings](https://github.com/tomhea/farray/tree/master/timings) folder contains c++ programs for measuring times and time-improvements of the header-files over an original array.
