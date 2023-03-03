# Short Description

Reading (at least the start of) this page is highly recommended before reading the [Advanced Features](Advanced-Features.md) page.

You can also choose to read the [Medium article](https://link.medium.com/Q8YbkDJX2bb) I wrote about this subject. It has more details, and it goes through some older implementations of the concept too.

The algorithm uses the lower part of the array (addresses 0-\[almost n\]) as blocks of adjacent cells.<br>
The last cells in the array, which can't form a block, are initialized and accessed normally.<br />
The 1 extra bit, called the flag, is set to 1 if the whole lower part of the array is written. Otherwise, it's set to 0.

If the flag is 1, then the array is fully written and can be accessed as a regular array.

### Digging Deeper (for flag==0)

A block consists of two half-blocks, each is a union of `|ptr|b|def|` and `value[]`.<br />
The lower part of the array is divided into 2 parts - UCA (lower) and WCA (upper).<br />
The UCA consists of the blocks 0-\[b-1\], while WCA consists of the blocks \[b-last_block\].<br />
b, and the current default value of the array, are saved in the |...|b|def| part of the last block.

Initialization is done block-wise, i.e. a whole block is initialized at once.<br />
Two blocks with indices (b1,b2) are considered *chained* if b1/b2 are in both UCA/WCA and <br />
`blocks[b1].firstHalf.ptr == b2 && blocks[b2].firstHalf.ptr == b1`.

>UCA-block is considered initialized if it is not chained,<br />
and the block's data is saved in the two half-blocks of the said block.

>WCA-block is considered initialized if it is chained,<br />
and the block's data is saved in the second-half-block of both the said block and its chained block.

>In that sense - every 2 chained blocks have exactly 1 written block, and that's the magic of it all.

When reading, the default value is returned for an uninitialized block, or the cell's data<br />
 (found in the same block, or the one chained to it) for an initialized one.<br />
When writing - we might increment b by one, in order to use an extra block (to chain it somehow).

---

More background and implementation details can be found in my [Medium article](https://link.medium.com/Q8YbkDJX2bb).
