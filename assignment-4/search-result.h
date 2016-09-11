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
 * Representation of search result struct:
 * word - ptr to dynamically allocated C-string
 * extrainfo - vector of extraInfoEntries
 */

typedef struct {
    char *word;
    vector extraInfo;
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
 * Representation of an article struct - 2 dynamically allocated C-strings
 */

typedef struct {
    char *title;
    char *url;
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
 * Comparison is done by the article's url
 */

int ArticleCompare(const void *elem1, const void *elem2);

/**
 * Type: extraInfoEntry
 * -------------
 * Representation of an extraInfoEntry struct - 2 ints denoting some article's ID and
 * a counter showing how many times a particular word shows up in the article
 */

typedef struct {
    int articleID;
    int counter;
} extraInfoEntry;

/**
 * Function: ExtraInfoEntryCompareByCounter
 * -----------------------------
 * Compares the two extraInfoEntries planted at the specified addresses.
 * Comparison is done by Counter
 *
 */

int ExtraInfoEntryCompareByCounter(const void *elem1, const void *elem2);

/**
 * Function: ExtraInfoEntryCompareByArticleID
 * -----------------------------
 * Compares the two extraInfoEntries planted at the specified addresses.
 * Comparison is done by ArticleID
 *
 */

int ExtraInfoEntryCompareByArticleID(const void *elem1, const void *elem2);


#endif
