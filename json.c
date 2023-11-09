// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// JSON-related module
//

// Implementation based on JSON standard https://datatracker.ietf.org/doc/html/rfc7159
// See examples of JSON at https://www.json.org/example.html (by douglas@crockford.com)


#include "vely.h"


//
// temporary string binding, zero-out byte with BINDV and restore with UNBINDV. Cannot be nested.
//
static char tb;
#define BINDV(x) tb=*(x); *(x) =0
#define UNBINDV(x) *(x) = tb
//
// end temporary string binding
//


// default number of json nodes allocated, incremented by
#define VV_JSON_NODES 32
// max depth of normalized name
#define VV_JSON_MAX_NESTED 32
// Add json node to list of nodes, if hash is used
// *i is the location of where this element was found, so ec is the location, and if cannot allocate memory, go to endj, which must
// be visible from where VV_ADD_JSON is used. j_tp is the type of node, j_str value, lc is the count in the list of nodes prior to
// this node, l is the list of nodes (normalized name). 'n' here is calculated if needed, which isn't needed if no-hash is used.
// So no-hash performs ultra-fast JSON parsing, with virtually no memory allocated.
// A node after the last has empty name.
#define VV_ADD_JSON(j_tp, j_str, lc, l) if (jloc->usehash) { vely_add_json(); char *n = vely_json_fullname (l, lc); if (n == NULL) { ec = *i; goto endj; } nodes[node_c].name = n; nodes[node_c].type = j_tp; nodes[node_c].str =  j_str; node_c++; }; if (jloc->node_handler != NULL) { l[lc].name = NULL; if ((*jloc->node_handler)(lc, l, j_str, j_tp) != VV_OKAY) { ec=*i; VELY_ERR0; errm = VV_ERR_JSON_INTERRUPTED; goto endj;} }

// Prototypes
static char *vely_json_fullname (json_node *list, num list_c);
static char *vely_json_num(char *val, num *rv);
static void vely_add_json ();
static num32 vely_get_hex(char *v, char **err);
static void vely_add_json_hash (vely_json *j);

// Variables used by json parser (recursive)
static json_node list[VV_JSON_MAX_NESTED]; // max depth of nested names, used to construct them
static num list_c = -1; // index of current node being traversed, it's index into array of json_node type, increments with {
static num node_tot = 0; //node total alloc'd
static num node_c = 0; //node counter (current)
static vely_jsonn *nodes = NULL; // nodes of normalized json (name, type, value)
static vely_json *jloc = NULL; // final result, super structure for the whole json document
static char nulled = 0; // character that is nulled to bind a string, used in the following parser iteration
static char *errm = ""; // error message
static num ec = -1; // location where error happened (0..size) or -1 if okay
static num depth = 0; // depth of recursion


//
// Get the normalized name of a leaf
// "list" is the array of names leading up to here, list_c is the index of the final name
// val is the value of leaf, type is its type
// returns normalized name for a leaf name in name:value, or NULL if too long a name, this is
// returned as an allocated value
// this accounts for any arrays
//
char *vely_json_fullname (json_node *list, num list_c)
{
    VV_TRACE("");


    if (list_c == 0) 
    { 
        char *fulln = vely_malloc (3); //  2 for "" + null 
        strcpy (fulln, "\"\""); 
        return fulln; 
    } // in case json doc is just a string or number and nothing else, so list_c is 0

    num i;
    num nlen = 0; // length of normalized name
    // first calculate the memory needed to hold normalized name
    for (i = 0; i < list_c; i++)
    {
        if (list[i].index == -1) 
        {
            nlen += 1 + 2 + list[i].name_len; /* 1 for dot, 2 for ", plus length of name (we ignore i==0 and just allocate one byte extra)*/
        }
        else
        {
            num a = list[i].index;
            num alen; // length of digits in array index
            if (a == 0) alen = 1; else { for (alen = 0; a != 0; alen++, a = a / 10){} }
            list[i].index_len = alen;
            nlen += 1 + 2 + list[i].name_len + 2 + list[i].index_len; /* 1 for dot, 2 for ", 2 for [], plus length of array index*/
        }
    }
    char *fulln = vely_malloc (nlen + 1); // +1 for null
    num fullnc = 0; // curr length of normalized name
    for (i = 0; i < list_c; i++)
    {
        // construct normalized name "x"[..]."y"...
        // first ."x" or "x" if first
        if (i != 0) { memcpy (fulln + fullnc, ".\"", 2); fullnc += 2; } // do not include leading dot, just 
                                                                        // in between nodes
        else { memcpy (fulln + fullnc, "\"", 1); fullnc += 1; }
        memcpy (fulln + fullnc, list[i].name, list[i].name_len); fullnc += list[i].name_len;
        memcpy (fulln + fullnc, "\"", 1); fullnc += 1;
        // then if array, add [xxx]
        if (list[i].index != -1) 
        {
            memcpy (fulln + fullnc, "[", 1); fullnc += 1;
            // output index number, first check for 0
            num al = list[i].index;
            if (al == 0) { fulln[fullnc] = '0'; fullnc += 1; }
            else
            {
                // here, get all digits, fill last first, since we're doing moduo 10
                num k = 0;
                while (al != 0)
                {
                    int r = '0' + (al % 10);
                    fulln[fullnc + list[i].index_len - 1 - k] = r;
                    k++;
                    al = al/10;
                }
                fullnc += k;
            }
            // finish with ]
            memcpy (fulln + fullnc, "]", 1); fullnc += 1;
        }
    }
    fulln[fullnc] = 0;
    return fulln;
}

//
// Set the end result of json parsing. 'j' is the json object, maxhash is the maximum size of
// hash table - by default it's 10000. It means hash table size will not be bigger than this, not
// that this many will be actually allocated!
// if usehash is true, do store in hash
// nodeh is a node-handler, i.e. pointer to function that handles nodes, NULL if none.
//
void vely_set_json (vely_json **j, num maxhash, char usehash, vely_json_node_handler nodeh)
{
    VV_TRACE("");
    // get json object
    *j = (vely_json*)vely_malloc (sizeof(vely_json));

    jloc = *j; // set local processing object

    // set details
    maxhash = (maxhash == -1 ? 10000:maxhash);
    if (maxhash < 10) maxhash = 10;
    jloc->maxhash = maxhash;
    jloc->dnext = 0; // index for traversing the whole document one by one
    jloc->hash = NULL; // hash for fast direct access
    jloc->usehash = usehash; // true if hash
    jloc->node_handler = nodeh; // node handler or NULL
}

//
// Delete all allocated data for json j
//
void vely_del_json (vely_json *j)
{
    VV_TRACE("");
    num i;
    for (i = 0; i < j->node_c; i++)
    {
        vely_free (j->nodes[i].name);
    }
    if (j->node_c != 0) vely_free (j->nodes);
    if (j->usehash) vely_delete_hash (&(j->hash), 0); // delete hash actually purges, but with 0 as second param, total deletion
                                // if new-json is called again, it will create new hash
    j->node_c = 0;
    vely_free (j); // delete the entire json structure
}

//
// Get JSON value from json "j" associated with "key" into "to". "type" is the type (VV_JSON_...), can be NULL. 
// Returns VV_OKAY on success, VV_ERR_EXIST on failure.
//
num vely_read_json (vely_json *j, char *key, char **keylist, char **to, num *type)
{
    VV_TRACE("");

    if (!j->usehash) return VV_ERR_EXIST; // no hash, no data
                                             //
    num st;
    vely_jsonn *n = (vely_jsonn*)vely_find_hash (j->hash, key, keylist, 0, &st, NULL);
    if (st == VV_ERR_EXIST) return VV_ERR_EXIST; // VELY_ERR0 done in vely_find_hash
    else 
    {
        *to = n->str;
        if (type != NULL) *type = n->type;
        return VV_OKAY;
    }
}

//
// Position to first json value for vely_next_json() (traverse clause in read-json)
//
void vely_begin_json (vely_json *j)
{
    VV_TRACE("");

    j->dnext = 0;
}

//
// Get next value from json j, into "key"/"to"/"type". 
// Return VV_OKAY is okay, VV_ERR_EXIST if no more.
//
num vely_next_json (vely_json *j, char **key, char **to, num *type)
{
    VV_TRACE("");

    if (!j->usehash) return VV_ERR_EXIST; // no hash, no data
                                             //
    if (j->dnext >= j->node_c) {VELY_ERR0;return VV_ERR_EXIST;}
    *key = j->nodes[j->dnext].name;
    *to = j->nodes[j->dnext].str;
    if (type != NULL) *type = j->nodes[j->dnext].type;
    j->dnext++;
    return VV_OKAY;
}

//
// Add json normalized names/values to a hash for fast retrieval
// 'j' is the json object
//
void vely_add_json_hash (vely_json *j)
{
    VV_TRACE("");
    num st;
    // create hash to add keys to, size it to match the document size, so close to 1 hit to get the key/value
    if (j->usehash) vely_create_hash (&(j->hash), j->node_c > j->maxhash ? j->maxhash : j->node_c, NULL, false);
    // go through all and add to hash
    num i;
    for (i = 0; i < j->node_c; i++)
    {
        // not checking for old value, always gets replaced
        // do not check for old key in hash, because we do not allocate the key (it's part of the document passed in)
        // so no need to free a duplicate key
        vely_add_hash (j->hash, j->nodes[i].name, NULL, (void*)&(j->nodes[i]), &(st), NULL);
        // generally should be one or the other, but should always succeed
        // if (st != VV_OKAY && st != VV_ERR_EXIST) vely_report_error ("Cannot add JSON text to internal hash");
    }

}

//
// Make sure json nodes always have room allocated for new elements
// As more elements are added, double the storage, up until 4K blocks
//
void vely_add_json ()
{
    VV_TRACE("");
    static num incby;
    
    if (node_tot == 0) incby = VV_JSON_NODES/2; // must start with half, so that initial block below is VV_JSON_NODES, since 
                                    // malloc/realloc choice depends on it
    if (node_c >= node_tot)
    {
        if (incby < 4096) incby *= 2; // initial block is VV_JSON_NODES
        node_tot += incby;
        if (node_tot == VV_JSON_NODES) nodes = vely_malloc (node_tot*sizeof (vely_jsonn));
        else nodes = vely_realloc (nodes, node_tot*sizeof (vely_jsonn));
        // initialize nodes to prevent program crashing if developer fails to check the status
        num i;
        for (i = node_c; i < node_tot; i++) {nodes[i].name = VV_EMPTY_STRING; nodes[i].str = ""; nodes[i].type = VV_JSON_TYPE_STRING; }
    }
}

//
// Returns current error or "" if none.
//
char *vely_json_err()
{
    return errm;
}

//
// parse val as JSON text. val is modified - make a copy of val if you still need it.
// len is the length of val, if -1 then we use strlen to figure it out
// curr is the current position from where to start parsing, it's NULL for top call in recursion
// returns -1 if okay, or position of error if not. 
// To get error, use vely_json_err()
// if "dec" is 0, do not decode strings, otherwise decode
//
num vely_json_new (char *val, num *curr, num len, char dec) 
{
    VV_TRACE("");

    char root_call = 0; // 1 if this is top call in recursive processing
    num c;
    num *i;
    if (curr == NULL) 
    { 

        // this is root call
        root_call = 1;
        errm = ""; // no error by default
        ec = -1; // exit code by default
        depth = 0; // max dept allowed, currently we're in first (root) invocation
        list_c = -1; // start with root for arrays

        // set byte counter to start from the beginning
        c = 0; 
        i = &c; 

        num j;
        for (j = 0; j < VV_JSON_MAX_NESTED; j++) 
        {
            list[j].index = -1; // array adds 1, so -1 means no array at this level
            list[j].name = NULL;
        }

        // create initial block of normalized nodes
        node_c = 0;
        node_tot = 0; // both node_c and node_tot must be 0 for allocation to work properly, see vely_add_json
        if (jloc->usehash) vely_add_json();

    } else i = curr; // inherit byte counter from a recursive parent

    // check if too many nested
    depth ++;
    if (depth >= VV_JSON_MAX_NESTED) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_DEPTH; goto endj; }
    
    // various flags for checking the validity of JSON doc, mostly what's expected to be found at any point
    char expected_comma_or_end_array = 0;
    char expected_comma_or_end_object = 0;
    char expected_colon = 0;
    char expected_name = 0; // when string is found, it's 1 if it's name, 0 for value. By default, we look for value.
    char isarr = 0; // 0 if not in array [], 1 if in array
    char isobj = 0; // 0 if not in object, 1 if in object

    //
    // JSON text is the same as value. So just "123" is a valid JSON text
    // A JSON value MUST be an object, array, number, or string, or one of the following three literal names: false null true
    //
    if (len == -1) len = (num) strlen(val); // len is -1 only in root invocation
    list_c++; // every time value is about to be found, go one level up (and when found, one down)
    // the limit for list_c is VV_JSON_MAX_NESTED -1 so there is always one empty after the last with empty name
    // to mark the end if key-count in read-json isn't used
    if (list_c >= VV_JSON_MAX_NESTED - 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_DEPTH; goto endj; }

    char nchar = 0;
    while (*i < len) // initial value of i is determined at the beginning of this function, which is recursive
    {
        if (nulled != 0) { nchar = nulled; nulled = 0; } else nchar = val[*i];
        switch (nchar) 
        {
            // begin object, zero or more name:value inside separated by commas. Names should be unique.
            case '{':
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                expected_name = 1;
                isobj = 1;
                (*i)++;
                break;

            // end object
            case '}':
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                expected_comma_or_end_object = 0;
                isobj = 0;
                list_c --; 
                (*i)++;
                { goto endj; }
                break;

            // begin array, zero or more values inside separated by commas. Values can be of different types.
            case '[':
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                isarr = 1;
                (*i)++; // get passed [ to find the value that follows
                // use previous element because array applies to it
                list_c--;
                list[list_c].index++; // if index was -1, now it's 0 (first element in array), otherwise increments it
                if (vely_json_new (val, i, len,dec ) != -1) { goto endj; }  
                // no incrementing *i because it's done in vely_json_new()
                expected_comma_or_end_array = 1;
                break;
            }
            // end array
            case ']':
                expected_comma_or_end_array = 0;
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                isarr = 0;
                (*i)++; // get passed ] to find the value that follows
                list[list_c].index = -1; // no longer array at this level
                list_c++; // increase to put it back where it was before we decreased it in [
                list_c --; 
                { goto endj; }
                break;

            // name:value separator
            case ':': 
            {
                expected_colon = 0;
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                (*i)++; // get passed : to find the value that follows
                if (vely_json_new (val, i, len ,dec) != -1) { goto endj; }  // return value if failed
                // no incrementing *i because it's done in vely_json()
                expected_comma_or_end_object = 1;
                break;
            }

            // value separator
            case ',':
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                expected_comma_or_end_array = 0;
                expected_comma_or_end_object = 0;
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                (*i)++; // get passed : to find the value that follows
                // must be within object or array
                if (isobj ==1) { expected_name = 1; continue;} // if we're in name:value list of pairs, continue to next name
                // if we're in array of values, find the next value
                else if (isarr == 1) list[list_c].index++; // this is next array element, advance the index
                else { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_UNRECOGNIZED; goto endj; }
                if (vely_json_new (val, i, len ,dec) != -1) { goto endj; }  // return value if failed
                // no incrementing *i because it's done in vely_json_new()
                if (isobj == 1) expected_comma_or_end_object = 1;
                if (isarr == 1) expected_comma_or_end_array = 1;
                break;
            }

            // white spaces 
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                (*i)++;
                continue;

            // string
            case '"': 
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                char *end;
                // get length of string
                end=vely_json_to_utf8 (val+*i, 1, &errm, dec);
                if (end == NULL) { goto endj; }
                int lstr = (end - (val+*i)); 
                BINDV(end); // put 0 at the end, do NOT set nulled because there's double quote 
                            // which we nulled and we don't want to continue from this double quote
                            // but rather from one byte ahead
                char *str = val + *i + 1;
                (*i) += lstr; // points to final 0, but *i gets increased in for() to get passed it
                if (expected_name == 1) 
                {
                    // this is name in an array of names leading up to name:value
                    list[list_c].name = str;
                    list[list_c].name_len = lstr - 1;
                    (*i)++;
                    expected_name = 0;
                    expected_colon = 1;
                }
                else
                {
                    //set node with value
                    VV_ADD_JSON(VV_JSON_TYPE_STRING, str, list_c, list);

                    (*i)++; // increase to get passed 0 byte when it returns
                    list_c --; 
                    { goto endj; }
                }
                // no UNBINDV, we modify original string at the point of closing " (we place 0 there)
                break;
            }
            // number
            case '-':
            case '0' ... '9': 
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                num rv;
                char *r = vely_json_num (val+*i, &rv);
                if (r == NULL) { ec = *i; goto endj; } // errm set in vely_json_num()
                char *str = val + *i;
                BINDV(r); // put 0 at the end
                nulled = tb;
                if (rv == 1) 
                {
                    //set node with value
                    VV_ADD_JSON(VV_JSON_TYPE_REAL, str, list_c, list);
                }
                else if (rv == 0) 
                {
                    //set node with value
                    VV_ADD_JSON(VV_JSON_TYPE_NUMBER, str, list_c, list);
                }
                else { { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NUMBER; goto endj; }}
                (*i) += (r - val-*i); 
                list_c --; 
                { goto endj; }

                break;

            // true
            case 't': 
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                if (strncmp (val+*i, "true", sizeof("true")-1)) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_UNKNOWN; goto endj; }
                char *str = val+*i;
                BINDV(val+*i+strlen("true")); // put 0 at the end
                nulled = tb;
                //set node with value
                VV_ADD_JSON(VV_JSON_TYPE_BOOL, str, list_c, list);

                (*i) += strlen("true"); // get passed value 
                list_c --; 
                { goto endj; }
                break;
            }

            // false
            case 'f':
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                if (strncmp (val+*i, "false", sizeof("false")-1)) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_UNKNOWN; goto endj; }
                char *str = val+*i;
                BINDV(val+*i+strlen("false")); // put 0 at the end
                nulled = tb;
                //set node with value
                VV_ADD_JSON(VV_JSON_TYPE_BOOL, str, list_c, list);

                (*i) += strlen("false"); // get passed value 
                list_c --; 
                { goto endj; }
                break;
            }
            // null
            case 'n':
            {
                if (expected_colon == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COLON_EXPECTED; goto endj; }
                if (expected_comma_or_end_array == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED; goto endj; }
                if (expected_comma_or_end_object == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED; goto endj; }
                if (expected_name == 1) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_NAME_EXPECTED; goto endj; }
                if (strncmp (val+*i, "null", sizeof("null")-1)) { ec = *i; VELY_ERR0; errm = VV_ERR_JSON_UNKNOWN; goto endj; }
                char *str = val+*i;
                BINDV(val+*i+strlen("null")); // put 0 at the end
                nulled = tb;
                //set node with value
                VV_ADD_JSON(VV_JSON_TYPE_NULL, str, list_c, list);

                (*i) += strlen("null"); // get passed value 
                list_c --; 
                { goto endj; }
                break;
            }
            
            default:
            {
                ec = *i; VELY_ERR0; errm = VV_ERR_JSON_UNRECOGNIZED; goto endj; // unrecognized token
            }
        }
    }
    list_c --; 

endj:    
    depth--;
    if (root_call == 1)
    {
        // final result, this is root call
        jloc->nodes = nodes;
        jloc->node_c = node_c;
        // add all elements to the hash - must be done now because adding pointers to nodes[] doesn't work
        // as nodes[] gets reallocated during parsing, and pointers are becoming invalid
        if (jloc->usehash) vely_add_json_hash (jloc);

        // parser always get json value, and there can be whitespace afterwards. After cleaning it up,
        // it must amount to the full document; otherwise there's something left-over that's not json.
        while (isspace (val[*i])) (*i)++; 

        // check if there's something extra not processed, but only if error not already set
        if (ec == -1 && *i < len)
        {
            ec = *i; VELY_ERR0; errm = VV_ERR_JSON_SYNTAX;
        }
    }
    return ec;
}

//
// rv is return value: 0 for number, 1 for double, 2 if bad
// returns pointer to first byte after number or NULL if failed. Sets errm.
//
char *vely_json_num(char *val, num *rv)
{
    VV_TRACE("");
    num i = 0;
    char isdbl = 0;
    
    // get the sign
    if (val[i] == '-')
    {
        i++;
    }
    
    // get the int, check if first char is digit and if 0, check there's nothing else
    num dig = i;
    while (isdigit(val[i])) i++;
    num edig = i;
    if (!isdigit(val[dig])) { VELY_ERR0; errm = VV_ERR_JSON_NUMBER; return NULL;}
    if (val[dig] == '0' && edig != dig+1) { VELY_ERR0; errm = VV_ERR_JSON_NUMBER; return NULL;}

    // get the decimal point
    if (val[edig] == '.')
    {
        i = edig + 1;
        // fraction start with dot to easily convert to double
        while (isdigit(val[i])) i++;
        // check there's at least one digit after dot
        if (!isdigit(val[edig+1])) { VELY_ERR0; errm = VV_ERR_JSON_NUMBER; return NULL;}
        isdbl = 1;
    }

    // get the exponent
    num exp = -1;
    if (val[i] == 'e' || val[i] == 'E')
    {
        i++;
        if (val[i] == '+') { i++;} 
        else if (val[i] == '-') { i++;} 
        exp = i;
        while (isdigit(val[i])) i++;
        // check there's at least one digit in exponent
        if (!isdigit(val[exp])) { VELY_ERR0; errm = VV_ERR_JSON_NUMBER; return NULL;}
        isdbl = 1;
    }

    if (isdbl == 0)
    {
        *rv = 0; // number
    }
    else if (isdbl == 1)
    {
        *rv = 1;
    }
    else
    {
        *rv = 2;
    }
    return val + i;
}

//
// Get value of string representing hex in 4 digits at 'v'. If error, err will be filled and returns 0.
// Return value of hex.
//
num32 vely_get_hex(char *v, char **err)
{
    VV_TRACE("");
    num k;
    num r = 0;
    for (k = 0; k < 4; k++)
    {
        if (*v >= '0' && *v <= '9') r += (*v - '0')*vely_topower(16,3-k);
            else if (*v >= 'a' && *v <= 'f') r += (*v - 'a' + 10)*vely_topower(16,3-k);
            else if (*v >= 'A' && *v <= 'F') r += (*v - 'A' + 10)*vely_topower(16,3-k);
            else { *err = VV_ERR_JSON_BADUTF; return 0;} // not a hex value
        v++;
    }
    return r;
}

//
// Obtain the string from json text "val", which starts with a first byte after double quote. 
// This string can be name or a string value in json. Sets errm.
// quoted is 1 if string is double quoted (as is with json)
// returns pointer to first byte after the string or NULL on error
// If 'enc' is 0, do not enccode strings at all.
//
char *vely_json_to_utf8 (char *val, char quoted, char **o_errm, char enc)
{
    VV_TRACE("");
    *o_errm = VV_EMPTY_STRING;
    num i = 1; // if quoted, start right after first char (which is a quote)
    if (quoted == 0) i = 0; // if not quoted, start from the beginning of string

    num pull = 0; // when interpreting escaped value, current tally of how many byte to copy current byte
                  // all val[] assignments subtract "pull" when being assigned

    if (enc == 0)
    {
        // this is no-encode
        while (val[i] != 0) // do not go past the end of string
        {
            if (val[i] == '\\' && val[i+1] != 0) i+=2; // if there is an escape, get passed both chars
            else if (val[i] == '"')  // found unescaped quote
            {
                // stop with unescaped quote, if our string is quoted, otherwise a quote is nothing special
                // and just continue until null char
                if (quoted == 1) break; else continue;
                i++;
            } else i++;
        }
    }
    else
    {
        // this is "encode", i.e. convert strings to binary format
        //
        // look for closing quote of end of string, or zero character. A quote inside is escaped and would be processed, so it would
        // not be caught here. Depending on whether quoted is 0 or 1, either one may be valid ending
        while (val[i] != '"' && val[i] != 0) 
        {
            // process escaped char
            if (val[i] == '\\')
            {
                i++; // move on to byte after the escape
                switch (val[i])
                {
                    // one byte escaped. we put in one bytes instead of two, so pull increases by 1
                    case '"': val[i-1-pull] = val[i]; pull++; break;
                    case '\\': val[i-1-pull] = val[i]; pull++; break;
                    case '/': val[i-1-pull] = val[i]; pull++; break;
                    case 'b': val[i-1-pull] = '\b'; pull++; break;
                    case 'f': val[i-1-pull] = '\f'; pull++; break;
                    case 'n': val[i-1-pull] = '\n'; pull++; break;
                    case 'r': val[i-1-pull] = '\r'; pull++; break;
                    case 't': val[i-1-pull] = '\t'; pull++; break;
                    // 5 bytes escaped for UTF-8 encoding
                    case 'u': 
                    {
                        num c = i;
                        num totjv = 6;
                        c++;
                        num r = vely_get_hex (val + c, o_errm);
                        if ((*o_errm)[0] != 0) return NULL;
                        num32 rtot;
                        if (r >= 0xD800 && r <= 0xDFFF)  
                        { 
                            totjv += 6;
                            c+=4; // get passed XXXX\u
                            if (val[c] != '\\' || val[c+1] != 'u')
                            {
                                VELY_ERR0;
                                *o_errm = VV_ERR_JSON_SURROGATE; return NULL;
                            }
                            num r1 = vely_get_hex (val + c + 2 , o_errm); // c+2 to skip \u
                            if ((*o_errm)[0] != 0) return NULL;
                            rtot = vely_make_from_utf8_surrogate (r, r1);
                        } else rtot = r;
                        // turn unicode to binary
                        num32 bytes = vely_decode_utf8 (rtot, (unsigned char*) (val+i-1-pull), o_errm);
                        if ((*o_errm)[0] != 0) return NULL;
                        pull += totjv - bytes; // binary is less space than \uXXXX 
                        i += totjv - 1 - 1; // 1 for getting passed \ and one for i++ further down
                        
                        break;
                    }
                    default: { VELY_ERR0; *o_errm = VV_ERR_JSON_BADESCAPE; return NULL;} // unknown escape sequence
                }
                i++; // to account for \, ", /, b, f, u etc.
            }
            else
            {
                // normal character, if no escapes just proceed to the end
                if (pull !=0) val[i-pull] = val[i];
                i++;
            }
        }
    }
    if (pull != 0) 
    {
        val[i-pull] = 0; // if pulled whole string back, finish it with 0
    }
    if (val[i] == 0 && quoted == 1) { VELY_ERR0; *o_errm = VV_ERR_JSON_NOQUOTE; return NULL;} // must have double quote at the end if quoted set
    return val+i; // return where the end is
}



//
// Convert binary utf8 val of len 'len' into json/unicode text 'resptr' (which is allocated here), 
// any error in err (also set here)
// If error, return -1, otherwise length of res
// len can be -1 in which case it's computed
//
num vely_utf8_to_json (char *val, num len, char **resptr, char **err)
{
    VV_TRACE("");

    *err = VV_EMPTY_STRING;
    if (len == -1) len = strlen (val);

    // allocate 3x memory, worst case scenario 2-byte utf8 to 6 byte unicode like \uXXXX
    // or 4-byte utf8 to 2x 6 byte surrogate unicodes. For others it's only 2x (such as \t)
    *resptr = (char*)vely_malloc(3*len + 1);
    char *res = *resptr;

    // note use of sprintf/like here is okay, as we have guaranteed enough space to write into (see above vely_malloc)
    num i;
    num r = 0;
    for (i = 0; i < len; i++)
    {
        if ((val[i] & 192) == 192)
        {
            // this is the beginning of utf8 sequence of bytes (2,3 or 4 bytes)
            // create \uXXXX (and possibly a surrogate pair)
            num32 u;
            num bytes = vely_encode_utf8 (val+i, &u, err);
            if (bytes == -1) return -1; else i+=(bytes-1); // since for loop will do i++
            if (u >= 0x10000) { // this means we need a surrogate pair
                num32 u0;
                num32 u1;
                vely_get_utf8_surrogate (u, &u0, &u1);
                //
                //Use direct memory manipulation instead of sprintf which is many times slower
                //
                //sprintf (res+r, "\\u%04x", u0);
                res[r] = '\\';
                res[r+1] = 'u';
                VV_HEX_FROM_INT16(res+r+2,u0);
                r+=6; // no need to set res[r] to 0, since we continue below
                //sprintf (res+r, "\\u%04x", u1);
                res[r] = '\\';
                res[r+1] = 'u';
                VV_HEX_FROM_INT16(res+r+2,u1);
                r+=6;
                res[r] = 0;
            }
            else
            {
                //
                //Use direct memory manipulation instead of sprintf which is many times slower
                //
                //sprintf (res+r, "\\u%04x", u);
                res[r] = '\\';
                res[r+1] = 'u';
                VV_HEX_FROM_INT16(res+r+2,u);
                r+=6;
                res[r] = 0;
            }
            continue;
        }
        else
        {
            switch (val[i])
            {
                // one byte escaped. we put in one bytes instead of two, so pull increases by 1
                case '"': memcpy (res+r, "\\\"", 2); r+=2; break;
                case '\\': memcpy (res+r, "\\\\", 2); r+=2; break;
                //
                // solidus is not encoded but for decoding it's recognized - this is common implementation
                //
                //case '/':  memcpy (res+r, "\\/", 2); r+=2;break;
                case '\b': memcpy (res+r, "\\b", 2); r+=2; break;
                case '\f': memcpy (res+r, "\\f", 2); r+=2; break;
                case '\n': memcpy (res+r, "\\n", 2); r+=2; break;
                case '\r': memcpy (res+r, "\\r", 2); r+=2; break;
                case '\t': memcpy (res+r, "\\t", 2); r+=2; break;
                default: { res[r++] = val[i]; } // just copy over
            }
        }
    }
    res[r] = 0;
    return r;
}


