#ifndef __searchresult_
#define __searchresult_
#include "vector.h"

/**
 * search-result.h
 * ---------------
 * Header for searchResultEntry structure and a helper article structure
 *
 */

/**
 * Type: searchResultEntry
 * ------------------
 * Representation of search result query.
 */

typedef struct {
    char *word;
    vector articles;
} searchResultEntry;

/**
 * Function: SearchResultEntryFree
 * -------------------------------
 * Properly disposes of the search result entry
 *
 * @param elem the address of the search result being freed.
 *
 */

void SearchResultEntryFree(void *elem);

/**
 * Function: SearchResultEntryCompare
 * -----------------------------
 * Compares the two searchResultEntries planted at the specified addresses.
 *
 * @param elem1 the address of a searchResultEntry
 * @param elem2 the address of a searchResultEntry, just like elem1.
 * @return an integer representing the difference between the ASCII values of the
 *         first non matching characters, or 0 if the two strings are equal.
 */

int SearchResultEntryCompare(const void *elem1, const void *elem2);

/**
 * Adapted version of the hash function provided by Julie Zelenski.
 *
 * @param elem a void * which is understood to be the address
 *             of a searchResultEntry.
 * @param numBuckets the number of buckets in the hash table.
 * @return the hashcode of the searchResultEntry addressed by elem.
 */

int SearchResultEntryHash(const void *elem, int numBuckets);

/**
 * Type: article
 * -------------
 * Representation of an article
 */

typedef struct {
    char *title;
    char *url;
    int counter;
} article;

/**
 * Function: ArticleFree
 * -------------------------------
 *
 * Properly disposes of the search result entry
 *
 * @param elem the address of the search result being freed.
 *
 */

void ArticleFree(void *elem);

/**
 * Function: ArticleCompare
 * -----------------------------
 * Compares the two articles planted at the specified addresses.
 * elem1 and elem2 are statically identified as void *s, but
 * we know that they're really char **s.  We cast and dereference
 * to arrive at char *s, and let strcmp do the traditional comparison
 * and use its return value as our own.
 *
 * @param elem1 the address of a , which itself addresses a null-terminated
 *              character array.
 * @param elem2 the address of a char *, just like elem1.
 * @return an integer representing the difference between the ASCII values of the
 *         first non matching characters, or 0 if the two strings are equal.
 */

int ArticleCompare(const void *elem1, const void *elem2);


#endif
