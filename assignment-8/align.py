#!/usr/bin/env python3

import random  # for seed, random
import sys  # for stdout


def find_optimal_alignment(dna1, dna2):
    """
    Computes the score of the optimal alignment of two DNA strands.
    :param dna1: DNA strand 1 - str of A, C, G, T
    :param dna2: DNA strand 2 - str of A, C, G, T
    :return: tuple with score of the optimal alignment and modified strands themselves
    """
    def find_optimal(strand1, strand2, memo):
        """
        helper recursive function
        :param strand1: DNA strand 1 - str of A, C, G, T
        :param strand2: DNA strand 2 - str of A, C, G, T
        :param memo: dict for memoization of recursive calls
        :return: tuple of (score, final strand1, final strand2)
        """
        # base case if one of the two strands is empty, then there is only
        # one possible alignment, and of course it's optimal
        len1, len2 = len(strand1), len(strand2)
        if len1 == 0:
            # score, strand1 leftover, strand2 leftover
            result = len2 * -2, ' ' * len2, strand2

            # remember call result
            memo[(strand1, strand2)] = result
            return result
        if len2 == 0:
            # score, strand1 leftover, strand2 leftover
            result = len1 * -2, strand1, ' ' * len1

            # remember call result
            memo[(strand1, strand2)] = result
            return result

        # There's the scenario where the two leading bases of
        # each strand are forced to align, regardless of whether or not
        # they actually match.

        # first, try getting result from memo
        try:
            best_with, best_strand1, best_strand2 = memo[(strand1[1:], strand2[1:])]
        except KeyError:
            best_with, best_strand1, best_strand2 = find_optimal(strand1[1:], strand2[1:], memo)

        if strand1[0] == strand2[0]:
            result = (best_with + 1,
                      strand1[0] + best_strand1,
                      strand2[0] + best_strand2)  # no benefit from making other recursive calls

            # remember call result
            memo[(strand1, strand2)] = result
            return result

        best = best_with - 1
        best_strand1 = strand1[0] + best_strand1
        best_strand2 = strand2[0] + best_strand2

        # It's possible that the leading base of strand1 best
        # matches not the leading base of strand2, but the one after it.

        # first, try getting result from memo
        try:
            best_without, best_without_strand1, best_without_strand2 = memo[(strand1, strand2[1:])]
        except KeyError:
            best_without, best_without_strand1, best_without_strand2 = find_optimal(strand1, strand2[1:], memo)

        best_without -= 2  # penalize for insertion of space
        if best_without > best:
            best_strand1 = ' ' + best_without_strand1
            best_strand2 = strand2[0] + best_without_strand2
            best = best_without

        # opposite scenario
        # first, try getting result from memo
        try:
            best_without, best_without_strand1, best_without_strand2 = memo[(strand1[1:], strand2)]
        except KeyError:
            best_without, best_without_strand1, best_without_strand2 = find_optimal(strand1[1:], strand2, memo)

        best_without -= 2  # penalize for insertion of space
        if best_without > best:
            best_strand1 = strand1[0] + best_without_strand1
            best_strand2 = ' ' + best_without_strand2
            best = best_without

        result = best, best_strand1, best_strand2

        # remember call result
        memo[(strand1, strand2)] = result
        return result

    memo_dict = {}
    return find_optimal(dna1, dna2, memo_dict)


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
        strand1 = generate_random_dna_strand(40, 60)
        strand2 = generate_random_dna_strand(40, 60)
        print("Aligning these two strands: " + strand1)
        print("                            " + strand2)
        alignment = find_optimal_alignment(strand1, strand2)
        print(alignment[1])
        print(alignment[2])
        print_alignment(alignment[0])


if __name__ == "__main__":
    main()
