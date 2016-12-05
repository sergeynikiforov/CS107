# Six degrees of Kevin Bacon

```
$ make
$ ./six-degrees
```

Propose two names, and the computer comes up with a sequence of movies and mutual co-stars
connecting the two:

```
Actor or actress [or <enter> to quit]: Jack Nicholson
Another actor or actress [or <enter> to quit]: Meryl Streep
		Jack Nicholson was in "Heartburn" (1986) with Meryl Streep.
```

Path to [data files](https://see.stanford.edu/materials/icsppcs107/assn-2-six-degrees-data.zip) is defined in `imdb-utils.h`

### [Overview](https://see.stanford.edu/materials/icsppcs107/09-Assignment-2-Six-Degrees.pdf)

There are two major components to this assignment:
* We need to provide the implementation for an `imdb` class, which allows us to
quickly look up all of the films an actor or actress has appeared in and all of the
people starring in any given film. We could layer our `imdb` class over two `STL`
maps — one mapping people to movies and another mapping movies to
people — but that would require we read in several megabytes of data from flat
text files. That type of configuration takes several minutes, and it’s the opposite of
fun if you have to sit that long before you play. Instead, we'll get use of the data 
representation in order to look up movie and actor information very, very quickly.
This is the meatier part of the assignment.
* We also need to implement a breadth-first search algorithm that consults our
`imdb` class to find the shortest path connecting any two
actor/actresses. If the search goes on for so long that we can tell it’ll be of length
7 or more, then you can be reasonably confident there’s no path connecting them.
This part of the assignment is a chance to get a little more experience with the STL and to
see a legitimate scenario where a complex program benefits from two types
paradigms: high-level C++ (with its templates and its object orientation) and lowlevel
C (with its exposed memory and its procedural orientation.)

