#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_ALLOC_SIZE 4

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
{ return 0; }

void *VectorNth(const vector *v, int position)
{ return NULL; }

void VectorReplace(vector *v, const void *elemAddr, int position)
{}

void VectorInsert(vector *v, const void *elemAddr, int position)
{}

void VectorAppend(vector *v, const void *elemAddr)
{}

void VectorDelete(vector *v, int position)
{}

void VectorSort(vector *v, VectorCompareFunction compare)
{}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ return -1; } 
