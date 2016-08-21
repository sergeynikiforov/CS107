#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_ALLOC_SIZE 4
#define ALLOC_MULTIPLIER 2

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(initialAllocation > 0);
    v->elemSize = elemSize;
    v->logicalLength = 0;
    v->allocatedLength = (initialAllocation == 0) ? DEFAULT_ALLOC_SIZE : initialAllocation;
    v->elems = malloc((v->allocatedLength) * elemSize);
    assert(v->elems != NULL);
    v->freeFunc = freeFn;
}

void VectorDispose(vector *v)
{
    if (v->freeFunc != NULL) {
        // iterate over all vector elements, calling custom free function
        char *cur = (char*) v->elems;
        char *end = ((char*) v->elems) + v->elemSize * v->logicalLength;
        while (cur != end) {
            v->freeFunc(cur);
            cur += v->elemSize;
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{ return v->logicalLength; }

void *VectorNth(const vector *v, int position)
{
    assert(position >= 0);
    assert(position < v->logicalLength);
    return (void*)(((char*) v->elems) + v->elemSize * position);
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position < v->logicalLength);

    void *oldElemAddr = ((char*) v->elems) + v->elemSize * position;

    // levy the custom free function if supplied
    if (v->freeFunc != NULL)
        v->freeFunc(oldElemAddr);

    memcpy(oldElemAddr, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position <= v->logicalLength);

    // check if a reallocation is needed
    if (v->allocatedLength == v->logicalLength) {
        v->allocatedLength *= ALLOC_MULTIPLIER;
        v->elems = realloc(v->elems, v->allocatedLength * v->elemSize);
        assert(v->elems != NULL);
    }

    // posAddr - where to put new element
    void *posAddr = ((char*) v->elems) + v->elemSize * position;

    // if we're not appending - move existing elements
    if (position < v->logicalLength) {
        // destAddr - where to move existing elements
        void *destAddr = ((char*) posAddr) + v->elemSize;
        memmove(destAddr, posAddr, (v->logicalLength - position) * v->elemSize);
    }

    // copy new element
    memcpy(posAddr, elemAddr, v->elemSize);
    v->logicalLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    void *destAddr;
    // check if a reallocation is needed
    if (v->allocatedLength == v->logicalLength) {
        v->allocatedLength *= ALLOC_MULTIPLIER;
        v->elems = realloc(v->elems, v->allocatedLength * v->elemSize);
        assert(v->elems != NULL);
    }
    destAddr = ((char*) v->elems) + v->elemSize * v->logicalLength;
    memcpy(destAddr, elemAddr, v->elemSize);
    v->logicalLength++;
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0);
    assert(position < v->logicalLength);

    // address of an element to delete
    void *elemAddr = ((char*) v->elems) + v->elemSize * position;

    // levy the custom free function if supplied
    if (v->freeFunc != NULL)
        v->freeFunc(elemAddr);

    // if we're deleting not the last element, move the elements
    if (position != v->logicalLength - 1) {
        // src - ptr to elements to move
        void *src = (char*) elemAddr + v->elemSize;
        memmove(elemAddr, src, (v->logicalLength - position - 1) * v->elemSize);
    }

    v->logicalLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->logicalLength, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);

    // iterate over the vector elements calling mapFn
    char *cur = (char*) v->elems;
    char *end = ((char*) v->elems) + v->elemSize * v->logicalLength;
    while (cur != end) {
        mapFn(cur, auxData);
        cur += v->elemSize;
    }
}

static const int kNotFound = -1;

int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert(startIndex >= 0);
    assert(startIndex <= v->logicalLength);

    void *startAddr = ((char*) v->elems) + v->elemSize * startIndex;
    void *foundAddr = NULL;

    // use binary search if the vector is sorted
    if (isSorted)
        foundAddr = bsearch(key, startAddr, v->logicalLength - startIndex, v->elemSize, searchFn);
    else {
        char *cur = (char*) startAddr;
        char *end = ((char*) v->elems) + v->elemSize * v->logicalLength;
        while (cur != end) {
            if (searchFn(key, cur) == 0) {
                foundAddr = (void*)cur;
                break;
            }
            cur += v->elemSize;
        }
    }
    return (foundAddr == NULL) ? kNotFound : ((char*) foundAddr - (char*) v->elems)/v->elemSize;
}
