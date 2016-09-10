#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "search-result.h"

// searchResultEntry
void SearchResultEntryFree(void *elem)
{
    searchResultEntry *entry = elem;
    free(entry->word);
    VectorDispose(&entry->articles);
}

int SearchResultEntryCompare(const void *elem1, const void *elem2)
{
    return strcmp(((searchResultEntry*)elem1)->word, ((searchResultEntry*)elem2)->word);
}

static const signed long kHash = -1664117991L;
int SearchResultEntryHash(const void *elem, int numBuckets)
{
    char *s = ((searchResultEntry*)elem)->word;
    unsigned long hashcode = 0;
    for (int i = 0; i < strlen(s); i++)
        hashcode = hashcode * kHash + tolower(s[i]);
    return hashcode % numBuckets;
}


// article
void ArticleFree(void *elem)
{
    article *a = elem;
    free(a->title);
    free(a->url);
}

int ArticleCompare(const void *elem1, const void *elem2)
{
    return strcmp(((article*)elem1)->url, ((article*)elem2)->url);
}
