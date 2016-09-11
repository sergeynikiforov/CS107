#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "search-result.h"

// allocation
/*
vector allocation;
int allocCompare(const void *elemAddr1, const void *elemAddr2)
{
    return (int)((int)*(unsigned int*)elemAddr1 - (int)*(unsigned int*)elemAddr2);
}
*/

// searchResultEntry
void SearchResultEntryFree(void *elem)
{
    searchResultEntry **entry = elem;
    free((*entry)->word);
    VectorDispose(&(*entry)->extraInfo);
    //free((*entry)->articles);
    free(*(void**)elem);
}

int SearchResultEntryCompare(const void *elem1, const void *elem2)
{
    return strcmp((*(searchResultEntry**)elem1)->word, (*(searchResultEntry**)elem2)->word);
}

static const signed long kHash = -1664117991L;
int SearchResultEntryHash(const void *elem, int numBuckets)
{
    char *s = (*(searchResultEntry**)elem)->word;
    unsigned long hashcode = 0;
    for (int i = 0; i < strlen(s); i++)
        hashcode = hashcode * kHash + tolower(s[i]);
    return hashcode % numBuckets;
}


// article
void ArticleFree(void *elem)
{
    //article **a = elem;
    //free((*a)->title);
    //free((*a)->url);
    /*
    int resTitle;
    if ((resTitle = VectorSearch(&allocation, &((*(article**)elem)->title), allocCompare, 0, true)) != -1) {
        // we can safely free
        printf("freeeee...\n");
        free((*(article**)elem)->title);
        VectorDelete(&allocation, resTitle);
    }
    int resUrl;
    if ((resUrl = VectorSearch(&allocation, &((*(article**)elem)->url), allocCompare, 0, true)) != -1) {
        // we can safely free
        printf("URLfreeeee...\n");
        free((*(article**)elem)->url);
        VectorDelete(&allocation, resUrl);
    }
    int resArticle;
    if ((resArticle = VectorSearch(&allocation, &(*(void**)elem), allocCompare, 0, true)) != -1) {
        // we can safely free
        printf("Articlefreeeee...\n");
        free((*(void**)elem));
        VectorDelete(&allocation, resArticle);
    }
    */
    free((*(article**)elem)->title);
    free((*(article**)elem)->url);
    free(*(void**)elem);
}

int ArticleCompare(const void *elem1, const void *elem2)
{
    return strcmp((*(article**)elem1)->url, (*(article**)elem2)->url);
}

// extraInfoEntry
int ExtraInfoEntryCompareByCounter(const void *elem1, const void *elem2)
{
    return ((extraInfoEntry*)elem1)->counter - ((extraInfoEntry*)elem2)->counter;
}

int ExtraInfoEntryCompareByArticleID(const void *elem1, const void *elem2)
{
    return ((extraInfoEntry*)elem1)->articleID - ((extraInfoEntry*)elem2)->articleID;
}
