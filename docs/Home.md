## Welcome to the Farray wiki!
**Farray** stands for **F**illable **Array** - an array with the `fill(v)` method,<br />
which sets all the values in the array to v.<br />
In this project, this method is O(1) time, and the array needs only 1 extra bit of space.<br />

This project is the implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper, written by Takashi Katoh & Keisuke Goto.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words.

We chose C++ for its *templates* and easy *manipulation of memory*, <br />
and header-only to make it simple for existing projects to include it.

The wiki features:
* [Short Despription of the paper and implementation](https://github.com/tomhea/farray/wiki/Short-Description)
* [Advanced Features](https://github.com/tomhea/farray/wiki/Advanced-Features)
* [Project Structure](https://github.com/tomhea/farray/wiki/Project-Structure)
* [Time and Space Analysis](https://github.com/tomhea/farray/wiki/Time-and-Space-Analysis)

The project is massively stress-tested, by the `.cpp` test files in the [tests](https://github.com/tomhea/farray/tree/master/tests) folder.

I wrote a [Medium article](https://link.medium.com/Q8YbkDJX2bb) about this project and array initialization in general. ~9 minutes of reading.

Enjoy!