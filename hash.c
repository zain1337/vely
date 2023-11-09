// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Hash-table-related module
//

#include "vely.h"


// prototypes
static num vely_compute_hash (char* d, char **dlist, num size);
static vely_hash_table *vely_new_hash_item (char *key, void *data);
void vely_del_hash_entry (vely_hash *h, vely_hash_table *todel, vely_hash_table *prev, num hashind);

//
// Create new hash hres_ptr. size is the size of hash table. The actual object is created here, the caller handlers pointer only.
// If process is true, then all memory allocated is process-scoped, and all key/data memory must be Vely memory.
// If in_h is provided, then vely_hash hres_ptr is not allocated, and neither is its vely_hash_table (which is set to in_h).
// Normally in_h is NULL; it's only non-NULL internally when building Vely internal hash values to find string values 
// quickly with just initialization of the table (see setup_reqhash() in v1.c).
//
void vely_create_hash (vely_hash **hres_ptr, num size, vely_hash_table **in_h, bool process)
{
    VV_TRACE("");
    // get object
    if (in_h == NULL) *hres_ptr = (vely_hash*)vely_malloc(sizeof(vely_hash));

    vely_hash *hres = *hres_ptr;

    if (size < 10) size = 10; // minimum size 10
    // create hash array of lists
    vely_hash_table **h;
    if (in_h == NULL) h = (vely_hash_table **)vely_calloc (size, sizeof(vely_hash_table*)); else h = in_h;

    // create top structure
    hres->num_buckets = size;
    hres->table = h; // set the table, each element is a linked list
    hres->tot = 0;
    hres->hits = 0;
    hres->reads = 0;
    hres->process = process;
    vely_rewind_hash(hres);// start from beginning when rewinding, in general index from which to keep dumping 

}

//
// Purges memory structure for hash h. Deletes the entire hash table. The actual object remains,
// so hash itself can be used again. If recreate is 1, then create new hash in its place, if not, do not (totally deleted).
// Total deletion (recreate is 0) happens with "delete" option of purge-hash
//
void vely_delete_hash (vely_hash **h, char recreate) 
{
    VV_TRACE("");
    if (*h == NULL || (*h)->table == NULL) return; // object isn't created yet
    num i;
    // loop through the table of linked lists
    for (i = 0; i < (*h)->num_buckets; i++) 
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
    num size = (*h)->num_buckets;
    bool process = (*h)->process;
    vely_free (*h); // delete old hash structure
    if (recreate == 1)
    {
        vely_create_hash (h, size, NULL, process); // create new hash
        // restore stats
        (*h)->hits = hits;
        (*h)->reads = reads;
    }
}


//
// Delete hash entry. todel is the entry to delete, prev is the pointer to previous entry,
// which can be NULL. The actual key/data must be freed (if needed) separately.
// hashind is the bucket index; if -1 we will calculate it; this avoids double calculation.
// h is the hash itself.
// hashind is the bucket index, or -1 if we need to compute it.
//
void vely_del_hash_entry (vely_hash *h, vely_hash_table *todel, vely_hash_table *prev, num hashind)
{
    vely_hash_table *next = todel->next;
    if (prev == NULL) {
        // if bucket index is unknown, calculate it since we need it when deleting the first in the bucket
        // in order to set 'prev' which is the bucket itself (i.e. denoted as NULL)
        if (hashind == -1) hashind = vely_compute_hash (todel->key, NULL, h->num_buckets);
        h->table[hashind] = next; // if first in bucket list deleted
                                                // the next one is now the first
    }
    else // if there is a one before the one to delete
    {
        prev->next = next; // update previous to point to the one after the deleted entry
                           // which can be NULL
    }
    vely_free (todel);
    // account for rewinding, if we just deleted the current element
    if (h->dcurr == todel)
    {
        h->dcurr = next; // set current to next, which can be NULL
    }

    h->tot--; // one element less
}

//
// Search / delete. Search hash 'h' for key 'key' and return data (or NULL if not found).
// If keylist!=NULL, then iterate over keylist array until NULL.
// If 'del' is 1, delete this element and return data (data is only linked). 'found' is VV_OKAY
// if something was found (say if delete was done), or VV_ERR_EXIST if not (found can be NULL).
// oldkey is the same as with vely_add_hash, see comments there.
//
void *vely_find_hash (vely_hash *h, char *key, char **keylist, char del, num *found, char **oldkey)
{
    VV_TRACE("");

    h->hits++;

    char *ret = NULL;
    // get hash id of a key
    num hashind = vely_compute_hash (key, keylist, h->num_buckets);

    vely_hash_table *hresult = NULL;
    vely_hash_table *prev = NULL;
    vely_hash_table *curr = h->table[hashind];
    
    while (curr != NULL)
    {
        h->reads++;
        bool is_match;
        if (keylist == NULL) is_match = !strcmp (key, curr->key);
        else
        {
            // Check if all elements in the list combined are the same as key
            num di = 0; // data index
            num lel; // length of element in key list
            num tot_len = 0; // current length compared in key
            char *el = keylist[di]; // element in key list
            if (el != NULL) 
            {
                is_match = true; // default, will set to false if not
                while (el != NULL) 
                {
                    // check if current element matches
                    lel = strlen (el);
                    if (memcmp (curr->key + tot_len, el, lel)) {is_match = false; break;}
                    // advance the position in key and get next element to check
                    tot_len += lel;
                    di++;
                    el = keylist[di];
                }
            } else is_match = false; // if first is NULL, no match
        }
        if (is_match)  { hresult = curr; break;}
        else
        {
            prev = curr;
            curr = curr->next;
        }
        
    }
    // if nothing here, not found, return NULL
    if (hresult == NULL) 
    { 
        if (found != NULL) {VELY_ERR0; *found = VV_ERR_EXIST;}
        if (oldkey != NULL) *oldkey = NULL; // no old key
        return NULL; // no old data
    }

    // save key,data before deleting, to be returned if asked for
    ret = hresult->data;
    char *okey = hresult->key;

    if (found != NULL) *found = VV_OKAY;
    if (del == 1) 
    {
        vely_del_hash_entry (h, hresult, prev, hashind);
    }
    if (oldkey != NULL) *oldkey = okey; // old key
    return ret; // old data
}

//
// Resize hash h based on newsize
//
void vely_resize_hash (vely_hash **h, num newsize)
{
    VV_TRACE("");

    // temp hash
    vely_hash *nh = NULL;
    vely_create_hash (&nh, newsize, NULL, (*h)->process);

    // copy data from old to new one using fast rewind
    vely_rewind_hash (*h);
    while (1)
    {
        void *data;
        char *key = vely_next_hash (*h, &data);
        if (key == NULL) break;
        vely_add_hash (nh, key, NULL, data, NULL, NULL);
    }
    vely_delete_hash (h, 1); // remove old hash, it also creates new one with old size, but empty
    vely_free ((*h)->table); // remove table created by default in vely_delete_hash()
    // copy back all the info about hash
    (*h)->table = nh->table;
    (*h)->tot = nh->tot;
    (*h)->hits = nh->hits;
    (*h)->reads = nh->reads;
    (*h)->num_buckets = nh->num_buckets;
    (*h)->process = nh->process;

    vely_free (nh); // release temp hash
}

//
// Rewind hash 'h' for getting all data.
//
void vely_rewind_hash(vely_hash *h)
{
    VV_TRACE("");
    h->dnext = 0;
    h->dcurr = h->table[h->dnext]; // can be NULL
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
    return h->num_buckets;
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
    if (h->dnext == h->num_buckets) { return NULL; }
    while (h->dcurr == NULL)
    {
        h->dnext++;
        if (h->dnext == h->num_buckets) { return NULL; }
        h->dcurr = h->table[h->dnext];
    }

    // get key and data
    *data = h->dcurr->data;
    char *key = h->dcurr->key;

    h->dcurr = h->dcurr->next; // current now
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
// Add new string 'data' to hash 'h' with key 'key'. If keylist!=NULL, then key is all keys in keylist until NULL.
// If this key existed, returns a pointer to old data, otherwise returns "" and st is VV_ERR_EXIST otherwise VV_OKAY
// (st can be NULL)
// oldkey returns the memory where the key actually stored in the hash is. While the value pointed to by this old key
// is the same as value pointed to by key are the same, the pointers may be different. This is useful in deleting old
// data and old key when replacing or deleting, because deleting hash entry means deleting hash constructs, not the actual data,
// after all the data may or may not be allocated. oldkey can be NULL, it's to be filled only if not NULL.
//
char *vely_add_hash (vely_hash *h, char *key, char **keylist, void *data, num *st, char **oldkey)
{
    VV_TRACE("");

    // compute hash id for key
    num hashind = vely_compute_hash (key, keylist, h->num_buckets);
    
    // check if process-scope, and if so, set memory to process
    // if process-key set, user guarantees key is process-scoped. Same for process-data
    // h->process is the same as vely_mem_process (set in v1.c to be equal)
    if (h->process && !vely_mem_process_key) _vely_mem_set_process (key);
    if (h->process && !vely_mem_process_data) _vely_mem_set_process (data);

    if (h->table[hashind] == NULL) { // nothing here, just add first list element 
        h->table[hashind] = vely_new_hash_item (key, data);
        h->table[hashind]->next = NULL; // no one following yet
        if (st != NULL) *st = VV_OKAY;
        h->tot ++; // one more element
        if (oldkey != NULL) *oldkey = NULL; // no old key
        return NULL;  // no old data
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
                if (st != NULL) {VELY_ERR0; *st = VV_ERR_EXIST;}
                // no increase of h->tot because this is replacement
                if (oldkey != NULL) *oldkey = bucket->key;
                bucket->key = key; // setup new key, and old key is retrieved, if asked
                                   // key (new key) and bucket->key (old key) point to the same value
                                   // but are two different pointers. We must replace old with new
                                   // or otherwise there would be leaks with unmanaged memory.
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
        if (oldkey != NULL) *oldkey = NULL; // no old key
        return NULL;
    }
}


#define VV_FNVPRIME 16777619
#define VV_FNVOFFSETBASIS 2166136261
//
// Compute hash value for string d. Size of hash table is size.
// If dlist!=NULL, iterate over dlist array until NULL.
// Return hash.
// Based on FNV1a for 32 bit, which is in public domain (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
// Based on http://www.isthe.com/chongo/tech/comp/fnv/index.html, this algorithm is not patented
// authors: fnvhash-mail@asthe.com
//
#define VV_FNVCOMP(h,c) h ^= (c); h *= VV_FNVPRIME;
inline num vely_compute_hash (char* d, char **dlist, num size)
{
    VV_TRACE("");
    uint32_t h = VV_FNVOFFSETBASIS;
    num i;
    // either use single value d or the list of keys dlist
    if (dlist == NULL) 
    {
        for (i = 0 ; d[i] ; i++)
        {
            VV_FNVCOMP(h, d[i]);
        }
    }
    else
    {
        // Go through key fragments to compute hash
        num dindex = 0;
        char *in;
        in = dlist[dindex];
        while (1)
        {
            if (in == NULL) break; // cut it short if key fragment is NULL
                                   // account if it's the first key fragment
                                   // this way
            for (i = 0 ; in[i] ; i++)
            {
                VV_FNVCOMP(h, in[i]);
            }
            dindex++;
            in = dlist[dindex]; 
        }
    }
    return (num)(h % size);
}





