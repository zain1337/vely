// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC.

// 
// Hash-table-related module
//

#include "vely.h"


// prototypes
static num vely_compute_hash (const char* d, num size);
static vely_hash_table *vely_new_hash_item (char *key, void *data);

//
// Create new hash hres_ptr. size is the size of hash table. The actual object is created here, the caller handlers pointer only.
//
void vely_create_hash (vely_hash **hres_ptr, num size)
{
    VV_TRACE("");
    // get object
    *hres_ptr = (vely_hash*)vely_malloc(sizeof(vely_hash));

    vely_hash *hres = *hres_ptr;

    if (size < 10) size = 10; // minimum size 10
    // create hash array of lists
    vely_hash_table **h = (vely_hash_table **)vely_calloc (size, sizeof(vely_hash_table*));
    // create top structure
    hres->size = size;
    hres->table = h; // set the table, each element is a linked list
    hres->tot = 0;
    hres->hits = 0;
    hres->reads = 0;
    vely_rewind_hash(hres);// start from beginning when rewinding, in general index from which to keep dumping 

}

//
// Purges memory structure for hash h. Deletes the entire hash table. The actual object remains,
// so hash itself can be used again. If recreate is 1, then create new hash in its place, if not, do not (totally deleted).
//
void vely_delete_hash (vely_hash **h, char recreate) 
{
    VV_TRACE("");
    if (*h == NULL || (*h)->table == NULL) return; // object isn't created yet
    num i;
    // loop through the table of linked lists
    for (i = 0; i < (*h)->size; i++) 
    { 
        // free all linked list elements
        vely_hash_table *hnext = (*h)->table[i];
        while (hnext != NULL)
        {
            vely_hash_table *tmp = hnext;
            hnext = hnext->next;
            vely_free (tmp);
        } 
    }
    vely_free ((*h)->table); // free table of linked lists
    // save stats
    num hits = (*h)->hits;
    num reads = (*h)->reads;
    vely_free (*h); // delete old hash structure
    if (recreate == 1)
    {
        vely_create_hash (h, (*h)->size); // create new hash
        // restore stats
        (*h)->hits = hits;
        (*h)->reads = reads;
    }
}

//
// Search / delete. Search hash 'h' for key 'key' and return data (or NULL if not found).
// If 'del' is 1, delete this element and return data (data is only linked). 'found' is 1
// if something was found (say if delete was done) (found can be NULL).
//
void *vely_find_hash (vely_hash *h, const char *key, char del, num *found)
{
    VV_TRACE("");

    h->hits++;

    char *ret = NULL;
    // get hash id of a key
    unsigned int hashind = vely_compute_hash (key, h->size);

    vely_hash_table *hresult = NULL;
    vely_hash_table *prev = NULL;
    vely_hash_table *curr = h->table[hashind];
    
    while (curr != NULL)
    {
        h->reads++;
        if (!strcmp (key, curr->key))  { hresult = curr; break;}
        else
        {
            prev = curr;
            curr = curr->next;
        }
        
    }
    // if nothing here, not found, return NULL
    if (hresult == NULL) { if (found != NULL) {VERR0; *found = VV_ERR_EXIST;} return NULL;}

    ret = hresult->data;

    if (found != NULL) *found = VV_OKAY;
    if (del == 1) 
    {
        vely_hash_table *next = hresult->next;
        if (prev == NULL) h->table[hashind] = next;
        else
        {
            prev->next = next;
        }
        vely_free (hresult);
        // account for rewinding, if we just deleted the current element
        if (h->dcurr == hresult)
        {
            h->dcurr = next; // set current to next, which can be NULL
        }
        h->tot--; // one element less
    }
    return ret;
}

//
// Resize hash h based on newsize
//
void vely_resize_hash (vely_hash **h, num newsize)
{
    VV_TRACE("");

    // temp hash
    vely_hash *nh = NULL;
    vely_create_hash (&nh, newsize);

    // copy data from old to new one using fast rewind
    vely_rewind_hash (*h);
    while (1)
    {
        void *data;
        char *key = vely_next_hash (*h, &data);
        if (key == NULL) break;
        vely_add_hash (nh, key, data, NULL);
    }
    vely_delete_hash (h, 1); // remove old hash, it also creates new one with old size, but empty
    vely_free ((*h)->table); // remove table created by default in vely_delete_hash()
    // copy back all the info about hash
    (*h)->table = nh->table;
    (*h)->tot = nh->tot;
    (*h)->hits = nh->hits;
    (*h)->reads = nh->reads;
    (*h)->size = nh->size;

    vely_free (nh); // release temp hash
}

//
// Rewind hash 'h' for getting all data.
//
void vely_rewind_hash(vely_hash *h)
{
    VV_TRACE("");
    h->dnext = 0;
    h->dcurr = h->table[h->dnext];
}

//
// Return average number of searches to get data
//
dbl vely_hash_reads (vely_hash *h)
{
    VV_TRACE("");
    if (h->hits == 0) return 0;
    return (dbl)(h->reads)/(dbl)(h->hits);
}

//
// Return how many buckets are in hash h
//
num vely_hash_size (vely_hash *h)
{
    VV_TRACE("");
    return h->size;
}

//
// Return how many elements are in hash h
//
num vely_total_hash (vely_hash *h)
{
    VV_TRACE("");
    return h->tot;
}

//
// Get next hash item from hash 'h'. 'data' is the value, and returns key.
// Returns NULL if no more (end of table). 
// This goes from ->dnext and to the next element, traversing all lists.
//
char *vely_next_hash(vely_hash *h, void **data) 
{
    VV_TRACE("");
    if (h->dnext == h->size) return NULL;
    while (h->dcurr == NULL) 
    {
        h->dnext++;
        if (h->dnext == h->size) return NULL;
        h->dcurr = h->table[h->dnext];
    } 

    *data = h->dcurr->data;
    char *key = h->dcurr->key;
    h->dcurr = h->dcurr->next;
    return key;
}


//
// Create new linked list item from key and data. Return pointer to it. 
//
vely_hash_table *vely_new_hash_item (char *key, void *data)
{
    VV_TRACE("");
    // create new hash linked list item
    vely_hash_table *new = (vely_hash_table *)vely_malloc (sizeof (vely_hash_table));
    // set data and key
    new->data = data; 
    new->key = key; 
    return new;
}

// 
// Add new string 'data' to hash 'h' with key 'key'
// If this key existed, returns a pointer to old data, otherwise returns "" and st is VV_ERR_EXIST otherwise VV_OKAY
// (st can be NULL)
//
char *vely_add_hash (vely_hash *h, char *key, void *data, num *st)
{
    VV_TRACE("");

    // compute hash id for key
    num hashind = vely_compute_hash (key, h->size);

    if (h->table[hashind] == NULL) { // nothing here, just add first list element 
        h->table[hashind] = vely_new_hash_item (key, data);
        h->table[hashind]->next = NULL; // no one following yet
        if (st != NULL) *st = VV_OKAY;
        h->tot ++; // one more element
        return NULL; 
    } 
    else 
    {
        // check if this key exists already, must be unique
        vely_hash_table *bucket = h->table[hashind];
        while (bucket != NULL)
        {
            if (!strcmp (key, bucket->key))
            {
                // match found
                char *old = bucket->data;
                bucket->data = data;
                if (st != NULL) {VERR0; *st = VV_ERR_EXIST;}
                // no increase of h->tot because this is replacement
                return old;
            } else bucket = bucket->next;
        }
        // save current head of the list
        vely_hash_table *el = h->table[hashind];
        // connect to old head of the list
        vely_hash_table *new =  vely_new_hash_item (key, data);
        new->next = el;
        // make it a new head of the list
        h->table[hashind] = new;
        if (st != NULL) *st = VV_OKAY;
        h->tot ++; // one more element
        return NULL;
    }
}


#define VV_FNVPRIME 16777619
#define VV_FNVOFFSETBASIS 2166136261
//
// Compute hash value for string d. Size of hash table is size.
// Return hash.
// Based on FNV1a for 32 bit, which is in public domain (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
// Based on http://www.isthe.com/chongo/tech/comp/fnv/index.html, this algorithm is not patented
// authors: fnvhash-mail@asthe.com
//
num vely_compute_hash (const char* d, num size)
{
    VV_TRACE("");
    uint32_t h = VV_FNVOFFSETBASIS;
    num i;
    for (i = 0 ; d[i] ; i++)
    {
        h ^= d[i];
        h *= VV_FNVPRIME;
    }
    return (num)(h % size);
}





