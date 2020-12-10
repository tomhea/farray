## Welcome to the Farray Website!
**Farray** stands for **F**illable **Array** - an array with the `fill(v)` method,<br />
which sets all the values in the array to v, in O(1) worst-time.<br />
Our implementation uses only 1 extra bit of memory, which is a state-of-the-art result.

This project is an implementation of the [In-Place Initializable Arrays](https://arxiv.org/abs/1709.08900) paper, written by Takashi Katoh & Keisuke Goto.

The paper is based on the simpler [Initializing an array in constant time](https://eli.thegreenplace.net/2008/08/23/initializing-an-array-in-constant-time) - which uses 2n extra memory words.

Both are explained in the [Medium article](https://link.medium.com/Q8YbkDJX2bb) I published about the subject and of course, in this website.

We chose C++ for its *templates* and easy *manipulation of memory*, <br />
and header-only to make it simple for existing projects to include it.

The Website features:
* [Short Despription of the paper and implementation](Short-Description.md)
* [Advanced Features](Advanced-Features.md)
* [Project Structure](Project-Structure.md)
* [Time and Space Analysis](Time-and-Space-Analysis.md)

The project is massively random-tested, by the _.cpp_ test files in the [tests](https://github.com/tomhea/farray/tree/master/tests) folder.

Enjoy!
