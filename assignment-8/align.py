#!/usr/bin/env python3

import random  # for seed, random
import sys  # for stdout


def find_optimal_alignment(strand1, strand2):
    """
    Computes the score of the optimal alignment of two DNA strands.
    :param strand1: DNA strand 1 - str of A, C, G, T
    :param strand2: DNA strand 2 - str of A, C, G, T
    :return: dict with score of the optimal alignment and modified strands themselves
    """
    def find_optimal(strand1, strand2, memo=None):
        """
        helper recursive function
        :param strand1: DNA strand 1 - str of A, C, G, T
        :param strand2: DNA strand 2 - str of A, C, G, T
        :param memo: dict for memoization of recursive calls
        :return: tuple of (score, final strand1, final strand2)
        """
        # base case if one of the two strands is empty, then there is only
        # one possible alignment, and of course it's optimal
        if len(strand1) == 0:
            # score, strand1 leftover, strand2 leftover
            return len(strand2) * -2, ' ' * len(strand2), strand2
        if len(strand2) == 0:
            # score, strand1 leftover, strand2 leftover
            return len(strand1) * -2, strand1, ' ' * len(strand1)

        # There's the scenario where the two leading bases of
        # each strand are forced to align, regardless of whether or not
        # they actually match.
        best_with, best_strand1, best_strand2 = find_optimal(strand1[1:], strand2[1:])
        if strand1[0] == strand2[0]:
            return (best_with + 1,
                    strand1[0] + best_strand1,
                    strand2[0] + best_strand2)  # no benefit from making other recursive calls

        best = best_with - 1
        best_strand1 = strand1[0] + best_strand1
        best_strand2 = strand2[0] + best_strand2

        # It's possible that the leading base of strand1 best
        # matches not the leading base of strand2, but the one after it.
        best_without, best_without_strand1, best_without_strand2 = find_optimal(strand1, strand2[1:])
        best_without -= 2  # penalize for insertion of space
        if best_without > best:
            best_strand1 = ' ' + best_without_strand1
            best_strand2 = strand2[0] + best_without_strand2
            best = best_without

        # opposite scenario
        best_without, best_without_strand1, best_without_strand2 = find_optimal(strand1[1:], strand2)
        best_without -= 2  # penalize for insertion of space
        if best_without > best:
            best_strand1 = strand1[0] + best_without_strand1
            best_strand2 = ' ' + best_without_strand2
            best = best_without

        return best, best_strand1, best_strand2

    return find_optimal(strand1, strand2)


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
    length = random.choice(range(minlength, maxlength + 1))
    bases = ['A', 'T', 'G', 'C']
    for i in range(length):
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
        print("Generate random DNA strands? ")
        answer = sys.stdin.readline()
        if answer == "no\n" or answer == "n\n":
            break
        strand1 = generate_random_dna_strand(8, 10)
        strand2 = generate_random_dna_strand(8, 10)
        print("Aligning these two strands: " + strand1)
        print("                            " + strand2)
        alignment = find_optimal_alignment(strand1, strand2)
        print(alignment[1])
        print(alignment[2])
        print_alignment(alignment[0])


if __name__ == "__main__":
    main()
