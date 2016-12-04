#!/usr/bin/env python

import random  # for seed, random
import sys  # for stdout


# Computes the score of the optimal alignment of two DNA strands.
def find_optimal_alignment(strand1, strand2):
    # if one of the two strands is empty, then there is only
    # one possible alignment, and of course it's optimal
    if len(strand1) == 0:
        return len(strand2) * -2
    if len(strand2) == 0:
        return len(strand1) * -2

    # There's the scenario where the two leading bases of
    # each strand are forced to align, regardless of whether or not
    # they actually match.
    best_with = find_optimal_alignment(strand1[1:], strand2[1:])
    if strand1[0] == strand2[0]:
        return best_with + 1  # no benefit from making other recursive calls

    best = best_with - 1

    # It's possible that the leading base of strand1 best
    # matches not the leading base of strand2, but the one after it.
    best_without = find_optimal_alignment(strand1, strand2[1:])
    best_without -= 2  # penalize for insertion of space
    if best_without > best:
        best = best_without

    # opposite scenario
    best_without = find_optimal_alignment(strand1[1:], strand2)
    best_without -= 2  # penalize for insertion of space
    if best_without > best:
        best = best_without

    return best


# Utility function that generates a random DNA string of
# a random length drawn from the range [minlength, maxlength]
def generate_random_dna_strand(minlength, maxlength):
    assert minlength > 0, \
        "Minimum length passed to generate_random_dna_strand" \
        "must be a positive number"
    assert maxlength >= minlength, \
        "Maximum length passed to generate_random_dna_strand must be at " \
        "as large as the specified minimum length"
    strand = ""
    length = random.choice(xrange(minlength, maxlength + 1))
    bases = ['A', 'T', 'G', 'C']
    for i in xrange(0, length):
        strand += random.choice(bases)
    return strand


# Method that just prints out the supplied alignment score.
# This is more of a placeholder for what will ultimately
# print out not only the score but the alignment as well.

def print_alignment(score):
    print("Optimal alignment score is ", score)


# Unit test main in place to do little more than
# exercise the above algorithm.  As written, it
# generates two fairly short DNA strands and
# determines the optimal alignment score.
#
# As you change the implementation of find_optimal_alignment
# to use memoization, you should change the 8s to 40s and
# the 10s to 60s and still see everything execute very
# quickly.

def main():
    while True:
        print ("Generate random DNA strands? ")
        answer = sys.stdin.readline()
        if answer == "no\n":
            break
        strand1 = generate_random_dna_strand(8, 10)
        strand2 = generate_random_dna_strand(8, 10)
        print("Aligning these two strands: " + strand1)
        print("                            " + strand2)
        alignment = find_optimal_alignment(strand1, strand2)
        print_alignment(alignment)


if __name__ == "__main__":
    main()
