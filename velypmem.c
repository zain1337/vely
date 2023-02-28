// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC. 

// 
// Memory handling add-on for super fast memory allocation. Works as fast, and measured
// even faster than stack allocation (probably due to CPU memory pre-fetching on a continuous block of memory)
// See velymem.c for more comments on general Vely strategy for memory.
//


#include "vely.h"

static num TOTMEM; // assigned by caller, this is how much memory will request attempt to grab for super fast access
static char *mem; // super fast memory
static int memt = 0; // top of memory
static char *mempp=NULL; // previously allocated mem, used to de-allocate the last known alloc


void *vely_fmalloc(int l);
void vely_ffree(void *);

//
// Fast malloc. Check if there is enough memory left in a single continuous block. If so use it.
// l is #bytes to allocate.
//
void *vely_fmalloc(int l) {
    if (l+sizeof(int) < TOTMEM-memt) {
        mempp=mem+memt+sizeof(int); // save last-known memory malloc-ed
        memt+=l+sizeof(int); // adjust the end of used memory (and beginning of free)
        *(int*)mempp=l;
        return mempp+sizeof(int); // return memory block asked
    }
    else return vely_malloc(l); // if not enough left, use regular Vely malloc
}


//
// Realloc memory. If using super fast block and this is the end of mem and enough space, just adjust end of used memory
// s is the memory to realloc, l is the new space
//
void vely_frealloc(void *s, int l) {
    long long d=(long long)((char*)s-mem); // distance to super fast memory beginning
    if (d>=0 && d<TOTMEM) { // if within super fast block
        if (s == mempp && l < TOTMEM-mempp) {
            memt=mempp-mem; // adjust end of used memory
            *(int*)(mempp-sizeof(int))=l;
            memt += l;
        } else {
            void *s_new = vely_malloc (l);
            memcpy (s_new, s, *(int*)(mempp-sizeof(int))); 
        }
    } else vely_free(s); // if not in super fast block, use Vely regular free
}


//
// Free memory. If using super fast block, just adjust end of used memory
// s is the memory to free.
//
void vely_ffree(void *s) {
    long long d=(long long)((char*)s-mem); // distance to super fast memory beginning
    if (d>=0 && d<TOTMEM) { // if within super fast block
        if (s == mempp) memt=mempp-mem-sizeof(int); // adjust end of used memory
    } else vely_free(s); // if not in super fast block, use Vely regular free
}

