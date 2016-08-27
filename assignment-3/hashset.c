#include "hashset.h"
#include <assert.h>
#include <stdio.h>
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
    h->numElements = 0;
    h->buckets = (vector**)malloc(numBuckets * (sizeof(vector*)));
    assert(h->buckets != NULL);

    // create empty vectors for all buckets
    vector **cur = h->buckets;
    vector **end = cur + numBuckets;
    while (cur != end) {
        // *cur is (vector*);
        *cur = malloc(sizeof(vector));
        VectorNew(*cur, elemSize, freefn, 4);
        cur++;
    }
}

void HashSetDispose(hashset *h)
{
    // iterate over all the buckets disposing the vectors
    vector **cur = h->buckets;
    vector **end = cur + h->numBuckets;
    while (cur != end) {
        VectorDispose(*cur);
        free(*cur);
        cur++;
    }

    // dispose the malloc'ed array of vector pointers
    free((void*)h->buckets);
}

int HashSetCount(const hashset *h)
{ return h->numElements; }

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    assert(mapfn != NULL);

    // iterate over all the stored elements
    vector **cur = h->buckets;
    vector **end = cur + h->numBuckets;
    while (cur != end) {
        VectorMap(*cur, mapfn, auxData);
        cur++;
    }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);

    // hash the element, determine the bucket
    int bucketNum = h->hashfn(elemAddr, h->numBuckets);
    assert(bucketNum >= 0);
    assert(bucketNum < h->numBuckets);

    // get the vector, try to find the element
    vector **vPtr = (h->buckets) + bucketNum;
    int searchRes = VectorSearch(*vPtr, elemAddr, h->comparefn, 0, true);

    // if nothing found, append the element, sort the vector, else - replace the element
    if (searchRes == -1) {
        VectorAppend(*vPtr, elemAddr);
        VectorSort(*vPtr, h->comparefn);
        h->numElements++;
    } else VectorReplace(*vPtr, elemAddr, searchRes);
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);

    // hash the element, determine the bucket
    int bucketNum = h->hashfn(elemAddr, h->numBuckets);
    assert(bucketNum >= 0);
    assert(bucketNum < h->numBuckets);

    // get the vector, try to find the element
    vector **vPtr = h->buckets + bucketNum;
    int searchRes = VectorSearch(*vPtr, elemAddr, h->comparefn, 0, true);

    // if nothing found, return NULL, else - addr on an element
    return (searchRes == -1) ? NULL : VectorNth(*vPtr, searchRes);
}
