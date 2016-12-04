# Tinkering With Python

```
$ python3 align.py
```

There are several alignment methods for measuring the similarity of two DNA
sequences (which for the purposes of this problem can be thought of as strings over a
four-letter alphabet: `A`, `C`, `G`, and `T`). One such method to align two sequences `x` and `y`
consists of inserting spaces at arbitrary locations (including at either end) so that the
resulting sequences `x'` and `y'` have the same length but do not have a space in the same
position. Then you can assign a score to each position. Position `j` is scored as follows:

* `+1` if `x'[j]` and `y'[j]` are the same and neither is a space, 
* `–1` if `x'[j]` and `y'[j]` are different and neither is a space,
* `–2` if either `x'[j]` or `y'[j]` is a space. 

The score for a particular alignment is just the sum of the scores over all positions.

The program `align.py` generates two random DNA strands and computes their optimal alignment, using
recursion and memoization of the intermediate results under the hood.

```
$ python3 ./align.py
Generate random DNA strands? 
yes
Aligning these two strands: ACTAGGTCCCTCAATGTAAAAGTTGACCAACGTTTACGGCCGAGGAAACTAGGAGCCT
                            TCCGTGGTTCCATACTCGTTAGGTTTCGGGGAAGTGGAGGGTAGTCGGCCGAGGGG

Optimal alignment score is -13

+  1   111 11  1 1 11 1   11 1   11 11  1 11  1  1   1 111 1   
  ACTA GGTCCCTCAAT GTAAAAGTT GACCAACGTTTACGGCCGAGGAAACTAGGAGCCT
  TCCGTGGTTCCATACTCGTTAGGTTTCGGGGAA GTGGAGGGTAGTCGGC CGAGGGG   
- 1 112   1  11 1 2  1 111  2 111  2  11 1  11 11 112 1   1 222

Generate random DNA strands? 
no
```

Please, use Python 3 for `align.py`.

`solution.pyc` is Python 2.4 only.