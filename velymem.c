// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC. 

// 
// Memory handling, including garbage collector
// VELY memory is made for request-based processing. Most of the time (if not all),
// memory is only allocated and at the end of a request, VELY will de-allocate 
// it. Always use vely_ functions, and unless absolutely needed, NEVER free
// memory allocated - it will be handled automatically at the end of the request.
// Uses stdlibc malloc, but keeps track of all memory
// usage. Memory is periodically released in entirety, eliminating memory fragmentation
// Release happens after servicing each request
//


#include "vely.h"

// If vely_mem_os is true, then OS alloc is used (free(), malloc() etc). This is in between
// guarded memory-cleanup off/on. One option was to add this memory to a current list. However, then
// freeing at the end of request would be doing more work by having to filter out these, for no good
// reason, other than to be able to "zap" all this memory at once. However, different requests may have
// different memories used for different purposes and a single "zap" seems unlikely and thus useless.
// For the same reason, no separate instances are used. This memory is not per "request name", it can be 
// used in any request, so separate instance based on request name would be useless too. Another option was
// to have "named" memory blocks that can be "zapped" based on name; similar issue. In addition, this all makes
// sense only with a single process; in multiple processes the data spaces are separate so any kind of name doesnt
// have much meaning anymore. So, basically, OS alloc remains very simple.
bool vely_mem_os = false;

// functions
VV_MEMINLINE num vely_get_memory (void *ptr);
char *vely_out_mem_mess = "Out of memory for [%lld] bytes";

// Common empty string constant, used for initialization
// When a variable has this value, it means it is freshly initialized and it
// will be either re-assigned, or allocated. (value assigned later)
char *VV_EMPTY_STRING=NULL;




#define VV_MEM_FREE 1
#define VV_MEM_FILE 2

// memory list
typedef struct s_vml {
    void *ptr;
    char status; // has VV_MEM_FREE bit set if this is freed block, 0 if not. 
                 // has VV_MEM_FILE bit set if this is a file that eventually needs to close (unless closed already)
    num next_free; // index to next freed block, could be anything by default
} vml;


// static variables used in memory handling
// We delete old's request memory at the very start of a new request (in generated code before any user code).
// Because of static designation here, no module can actually directly read the memory (of itself or other modules).
//
static vml *vm = NULL;
static num vm_curr = 0;
static num vm_tot = 0;
static num vm_first_free = -1; // first free memory block
static bool vm_mem_type = false; // used to save managed/unmanaged and restore                               


// determines the size of the block allocated (and the size of consequent expansions) for the memory
// block that keeps all pointers to allocated blocks.
#ifdef DEBUG
//easier to find out problems if smaller when debugging
#define VELYMSIZE 128
#else
#define VELYMSIZE 512
#endif

// 
// must be called at the very beginning of program. Initializes memory and 
// cleans up any memory left over from a previous request.
//
void vely_memory_init ()
{
    // setup "non-initialized memory" (neither NULL nor initialized)
    if (VV_EMPTY_STRING == NULL)
    {
        // this memory is never freed; remains for the life of process
        VV_EMPTY_STRING = (char*)malloc(1);
        if (VV_EMPTY_STRING == NULL) vely_report_error ("Out of memory");
        VV_EMPTY_STRING[0] = 0;
    }

    // release of previous request's memory and creation of new memory for the following reques
    // is done here; vely_memory_init() is called at the end of any request.
    vely_done ();

    vm = calloc (vm_tot = VELYMSIZE, sizeof (vml));
    if (vm == NULL) vely_report_error ("Out of memory");
    vm_curr = 0;
    vm_first_free = -1;
}

//
// Set ptr for mem entry to NULL. This is when memory was freed (such as when FILE* was closed)
// so the step to do this in vely_done() on request ending can be skipped.
//
VV_MEMINLINE void vely_clear_mem (num ind)
{
    vm[ind].ptr = NULL;
}

//
// Set status of memory, s is the status such as VV_MEM_FILE
// p is the new pointer (realloced).
//
VV_MEMINLINE void vely_set_mem_status (unsigned char s, num ind)
{
    vm[ind].status |= s;
}

// 
// Add point to the block of memory. 'p' is the memory pointer (allocated elsewhere here) added.
// Returns the index in memory block where the pointer is.
// Once a block of pointers is exhausted, add another block. We do not increase the blocks
// size size requests are generally small and typically do not need much more memory, and increasing
// the block size might cause swaping elsewhere.
//
VV_MEMINLINE num vely_add_mem (void *p)
{
    num r;
    if (vm_first_free != -1)
    {
        r = vm_first_free;
        vm_first_free = vm[vm_first_free].next_free; // this block if free for sure b/c vm_first_free points to it
                                                     // next_free is -1 if there's no next, and -1 is vm_first_free when none
    } else {
        r = vm_curr;
        vm_curr++;
        if (vm_curr >= vm_tot)
        {
            num old_vm_tot = vm_tot;
            vm_tot += VELYMSIZE;
            vm = realloc (vm, vm_tot * sizeof (vml));
            if (vm == NULL)
            {
                vely_report_error (vely_out_mem_mess, vm_tot*sizeof(vml));
            }
            // initialize memory status, this is done automatically in calloc, but NOT in realloc
            while (old_vm_tot != vm_tot) vm[old_vm_tot++].status = 0;
        }
    }
    vm[r].ptr = p;
    vm[r].status &= ~VV_MEM_FREE; // not a freed block
    return r;
}

// 
// Adds pointer to our block of memory. 'p' is the pointer allocated elsewhere.
// 'r' is the index in the block of memory where p is. 
// The memory returned is the actually a pointer to useful memory (that a VELY program can use). We place
// some information at the beginning of the memory pointed to by 'p': the reference to the 
// index in the block of memory where p is;
//
VV_MEMINLINE void *vely_vmset (void *p, num r)
{
    // num must be aligned to 8 byte bounder, which is fine since malloc'd mem is 16-byte aligned
    memcpy ((unsigned char*)p, &r, sizeof (num));
    return (unsigned char*)p + VV_ALIGN;
}




// 
// __ functions are top-level function that are actually called through macros
//

// 
// input and returns are like malloc().
//
VV_MEMINLINE void *_vely_malloc(size_t size)
{
    if (vely_mem_os) 
    {
        void *p = malloc(size);
        if (p == NULL) 
        {
            vely_report_error (vely_out_mem_mess, size);
        } else return p;
    }
    size_t t;
    void *p = malloc (t=size + VV_ALIGN);
    if (p == NULL) 
    {
        vely_report_error (vely_out_mem_mess, size+VV_ALIGN);
    }
    // add memory pointer to memory block
    num r = vely_add_mem (p);
    return vely_vmset(p,r);
}

// 
// input and returns are like calloc()
// See malloc for the rest
//
VV_MEMINLINE void *_vely_calloc(size_t nmemb, size_t size)
{
    if (vely_mem_os) 
    {
        void *p = calloc(nmemb, size);
        if (p == NULL) 
        {
            vely_report_error (vely_out_mem_mess, nmemb*size);
        } else return p;
    }
    size_t t;
    void *p =  calloc (1, t=(nmemb*size + VV_ALIGN));
    if (p == NULL) 
    {
        vely_report_error (vely_out_mem_mess, t);
    }
    num r = vely_add_mem (p);
    return vely_vmset (p,r);
}

//
// Get index of pointer in memory block
//
VV_MEMINLINE num vely_get_memory (void *ptr)
{
    return *(num*)((unsigned char*)ptr-VV_ALIGN);
}


// 
// Input and return the same as for realloc()
// Checks memory to make sure it's valid block allocated here.
// if safe is 1, perform additional checks to make sure it's okay memory
//
VV_MEMINLINE void *_vely_realloc(void *ptr, size_t size, char safe)
{
    size_t t;
    //
    // Check if string uninitialized, if so, allocate it for the first time
    // Also, if pointer is a NULL ptr, just allocate memory.
    //
    if (ptr == VV_EMPTY_STRING || ptr == NULL)
    {
        return _vely_malloc (size);
    }

    // if OS mem (i.e. not vely mem)
    if (vely_mem_os) 
    {
        void *p = realloc(ptr, size);
        if (p == NULL) 
        {
            vely_report_error (vely_out_mem_mess, size);
        } else return p;
    }


    num r = vely_get_memory(ptr);
    if (safe) 
    {
        if ((r < 0 || r >= vm_curr) || vm[r].ptr != ((unsigned char*)ptr-VV_ALIGN)) vely_report_error("Invalid memory to realloc");
    }
    vm[r].ptr = NULL;
    void *p= realloc ((unsigned char*)ptr-VV_ALIGN, t=size + VV_ALIGN);
    if (p == NULL) 
    {
        vely_report_error (vely_out_mem_mess, size+VV_ALIGN);
    }
    r = vely_add_mem(p);
    return vely_vmset(p,r);
}

//
// Free memory, but if memory violated, just ignore the error and continue
//
VV_MEMINLINE num vely_safe_free (void *ptr)
{
    // this check will either catch error or SIGSEGV
    if (!_vely_free (ptr, 1)) {VERR0; return VV_ERR_MEMORY; } // memory not valid
    return VV_OKAY;
}

// 
// Similar to free(), returns true if okay, false if not. If check is 1, it 
// checks if memory is valid. This isn't foolproof, because it may sigseg if memory outside
// of process memory space.
// Checks memory to make sure it's valid block allocated here.
//
VV_MEMINLINE bool _vely_free (void *ptr, char check)
{
    //
    // if programmer mistakenly frees up VV_EMPTY_STRING, just ignore it
    // this is true whether using vely mem or OS mem
    //
    if (ptr == VV_EMPTY_STRING || ptr == NULL) return true;

    // if OS mem (i.e. not vely mem)
    if (vely_mem_os) { free(ptr); return true;}

    //
    // Any point forward code can sigegv in this function or return false to indicate memory is bad
    //
    num r = vely_get_memory(ptr); // index in the list of memory blocks

    //
    // check is 1 when delete-mem is used
    //
    if (check == 1) // this is when delete-mem is used. Check if memory is okay. When we
                    // internally free memory, it should work (or it's a bug)
    {
        //
        // Check if the pointer is valid. At this point it's either sigsegv or the pointers do
        // not match.
        //
        if (r < 0 || r >= vm_curr) { return false;} 
        if (vm[r].ptr != ((unsigned char*)ptr-VV_ALIGN)) {return false;}
        //
        // Now we know the pointer is valid (very high probability, chances of random point in memory having
        // the exact pointer to data passed to here are practially zero).
        //
    }

    //
    // if memory on the list of freed blocks, just ignore it, otherwise double free or free
    // of an invalid pointer ensues. But do return an issue.
    //
    if (vm[r].status & VV_MEM_FREE) return false;

    // free mem
    vm[r].ptr = NULL;
    vm[r].status |= VV_MEM_FREE; // freed
    free ((unsigned char*)ptr-VV_ALIGN);
    // Set memory marked as freed, would be -1 if there's no free blocks at this moment
    vm[r].next_free = vm_first_free;
    vm_first_free = r;
    //
    return true; // free succeeded
}

// 
// Input and return the same as for strdup()
//
VV_MEMINLINE char *_vely_strdup (char *s)
{
   num l = strlen (s);
   char *n = (char*)_vely_malloc (l+1);
   if (n == NULL) 
   {
       vely_report_error (vely_out_mem_mess, l+1);
   }
   memcpy (n, s, l+1);
   return n;
}

// 
// Frees all memory allocated so far.
// This is called at the beginning of a request before memory is allocated again.
// The reason this is NOT called at the end of the request is that web server NEEDS
// some of the allocated memory even after the request ends, for example, the 
// actual web output or header information.
//
void vely_done ()
{
    if (vm != NULL)
    {
        num i;
        for (i = 0; i < vm_curr; i++)
        {
            // if already freed, continue; we can do such a check prior to actually freeing since
            // we have the index into vm[] here
            if (vm[i].status & VV_MEM_FREE) continue; 
                                                                        
            if (vm[i].ptr != NULL)
            {
                //VV_TRACE("Freeing [%lld] block of memory", i);
                if (vm[i].status & VV_MEM_FILE)
                {
                    FILE **f = (FILE**)(vm[i].ptr);
                    if (*f != NULL) fclose (*f);
                    *f=NULL; // make sure it's NULL so the following request can use this file descriptor
                }
                else
                {
                    _vely_free ((unsigned char*)vm[i].ptr+VV_ALIGN, 0);
                }
            }
        }
        //VV_TRACE("Freeing vm");
        free (vm);
        vm = NULL;
    }
}

void vely_unmanaged() {vm_mem_type = vely_mem_os; vely_mem_os = true;}
void vely_managed() {vm_mem_type = vely_mem_os; vely_mem_os = false;}
void vely_mrestore() {vely_mem_os = vm_mem_type;}





