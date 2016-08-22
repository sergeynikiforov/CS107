#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
    assert(hashfn != NULL);
    assert(comparefn != NULL);
    assert(numBuckets > 0);
    assert(elemSize > 0);
    h->elemSize = elemSize;
    h->numBuckets = numBuckets;
    h->hashfn = hashfn;
    h->comparefn = comparefn;
    h->freefn = freefn;
    h->buckets = malloc(numBuckets * sizeof(vector*));
    // create empty vectors for all buckets
    vector **cur = h->buckets;
    vector **end = cur + numBuckets;
    while (cur != end) {
        VectorNew(*cur, elemSize, freefn, 2);
        cur++;
    }
}

void HashSetDispose(hashset *h)
{}

int HashSetCount(const hashset *h)
{ return 0; }

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{}

void HashSetEnter(hashset *h, const void *elemAddr)
{}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ return NULL; }
