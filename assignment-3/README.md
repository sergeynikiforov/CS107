# `vector` and `hashset`

### The C `vector`

The C `vector` is a more flexible extension of C's built-in array type. It has some of the same
basic properties: it is managed as a contiguous region of memory, all the elements in any
one array must be the same size, and element indexing starts at 0, not 1. But this C `vector`
differs in that it can automatically grow when elements are appended, and it can splice
elements in and out of the middle. And through the use of client-supplied function pointers,
it can sort itself, iterate over its elements, and search for an element of interest.

### The C `hashset`

`hashset` is designed to imitate the C++ hash_set without all of the code bloat that comes with templates.
When initializing a hashset, the client specifies the element size and the number of buckets.
Because the hashset implementation has no knowledge about client element structure, the
client must also supply the hash function and comparison function needed to place
elements. Hash collisions are resolved by chaining (i.e. whenever two elements
collide, they both belong in the same bucket).

The `thesaurus-lookup.c` and `streaktokenizer.c` files, when
compiled against fully operational versions of `vector` and `hashset`,
contribute to an application called `thesaurus-lookup`, which is
this C-string intense application that loads a thesaurus into
a hashset (where synonym sets are stored in vectors) and prompts user to enter
a searching query.

```
$ make
$ ./thesaurus-lookup 
Loading thesaurus. Be patient! .............................. [All done!]
Go ahead and enter a word: world
We found "world" in the thesaurus! Its related word of the day is "sidereal universe".
```

