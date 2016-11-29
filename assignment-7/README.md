# Where am I?

Here is the problem: you are in a two-dimensional space, and you are lost. Although
you don't know your location, you do have a fairly accurate map that indicates where all
the stars are in the space. To sort things out, you measure the approximate distance to
several of the stars around you. Using the map and the distance measurements, figure
out where you are.

Given a list of distances and a list of star locations, the function where-am-i
computes all the possible guesses. The result is a list of rated points.
The first point is where you are, the rest are your other possible
locations, in decreasing order of likelihood.

```
> (where-am-i '(2.5 11.65 7.75) '((0 0) (4 4) (10 0))) 
((2.6667957200611248e-11 (11.481441859657613 2.0012201104648017))
 (0.6196504622642723 (2.128838984322123 0.5892556509887895))
 (1.2791764389328528 (-1.4000624999999995 -1.5820248862529624))
 (19.346163860515972 (3.9811874999999994 6.126803974552016))
 (85.14877936158665 (3.3410328105496716 3.4572187079855694))
 (3592.3637169904086 (5.801628692776499 -2.4937120261098316)))
> (where-am-i '(1 2) '((0 0) (3 0))) 
((1 (1 0)) (1 (2 0)))
```

`#lang scheme` at the top of the files is needed for developing with DrRacket environment.

