// SPDX-License-Identifier: EPL-2.0
// Copyright 2023 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Tree implementation, in-memory modified AVL with optional double-linked sorted list. 
// Around 50% faster than in-memory B-tree variations.
//

#include "vely.h"


// connect parent (p) to child (r) given the direction from the tree node (dir)
#define VV_TREE_SET_PARENT(p, dir, r) if (dir == VV_TREE_LESSER) (p)->lesser_node=(r); else (p)->greater_node=(r)

vely_tree_cursor *vv_cursor; // internal cursor for the current tree operation

// Function prototypes for the implementation
void vv_tree_rotate_left (vv_tree_node *parent_tree, int dir, vv_tree_node *tree);
void vv_tree_rotate_right (vv_tree_node *parent_tree, int dir, vv_tree_node *tree);
void vv_tree_insert(vv_tree_node *parent_tree, int dir, vv_tree_node *tree, void *data);
int vv_tree_compare(char *k2);
void vv_tree_search (vv_tree_node *tree);
void vv_tree_height (vv_tree_node *tree, num *factor);
void vv_tree_show (vv_tree_node *tree, num ident);
void vv_tree_balance (vv_tree_node *parent_tree, int dir, vv_tree_node *tree);
void vv_tree_delete (vv_tree_node *parent_tree, int dir, vv_tree_node *tree);
void vv_tree_find_leaf_del (vv_tree_node *parent_tree, int dir, vv_tree_node *tree_greater_node, vv_tree_node *found);
void vv_tree_search_lesser_equal (vv_tree_node *tree, bool equal);
void vv_tree_search_greater_equal (vv_tree_node *tree, bool equal);
vv_tree_node *vv_tree_node_create(char sorted);
void vv_tree_node_delete(vv_tree_node *tree);

// measuring tree hops (i.e. the cost to search), only for debug build
#ifdef DEBUG
#define VV_TREE_HOPS    vv_cursor->root->hops++,
#else
#define VV_TREE_HOPS    
#endif

// Evaluate the tree node key k2 with another fixed key obtained from vv_cursor->key, 
// i.e. compare fixed key<=>current_tree_node_key. Uses custom eval function if specified
#define VV_TREE_EVAL(k2) (VV_TREE_HOPS vv_cursor->root->eval?vv_cursor->root->eval(k2):vv_tree_compare(k2))

// Default key evaluation function. Compares current tree node key with vv_cursor->key. Return -1, 0, or 1 if node-key lesser, equal or greater than given key.
// Note: POSIX actually specifies that strncmp() works if the length compared is greater than the length of one of the strings (memcmp doesn't do that)
// vv_cursor holds the key and key_len members, which are precomputed. k2 is the tree node key.
inline int vv_tree_compare(char *k2)
{
    VV_TRACE("");
    if (vv_cursor->root->key_type == VV_TREE_TYPE_NUM)
    {
        // key-as positive-integer, compare as numbers written as strings, much faster than atoll()
        // works in any base
        num l1 = vv_cursor->key_len;
        num l2 = strlen(k2);
        if (l1<l2) return -1;
        if (l1>l2) return 1;
        return strncmp(vv_cursor->key,k2,l1);
    }
    else
    {
        // compare as classic strings, C collation
        char *k1=vv_cursor->key;
        num res = strncmp (k1,k2, vv_cursor->key_len);
        if (res < 0) return -1;
        if (res == 0)
        {
            if (k2[vv_cursor->key_len] != 0) return -1; else return 0;
        } else return 1;
    }
}

//
// Delete tree node. Actual key and data must be obtained prior and deleted if needed. tree is the node to delete.
//
void vv_tree_node_delete(vv_tree_node *tree)
{
    VV_TRACE("");
    vely_free (tree);
}

//
// Create tree node. Allocated pointers for linked list if sorted is 1. Returns the node.
//
vv_tree_node *vv_tree_node_create(char sorted)
{
    VV_TRACE("");
    vv_tree_node *res;
    res = vely_calloc (1, sizeof(vv_tree_node) + (sorted==1 ? 2*sizeof (vv_tree_node *):0));
    return res;
}


//
// Create root node of the tree. 
// res is the tree itself, sorted is true/false based on unsorted clause in new-tree.
//
void vv_tree_create_root (vely_tree *res, bool sorted) 
{
    VV_TRACE("");
    res->root_node = vv_tree_node_create(sorted?1:0); // never used directly, only reference as tree->lesser, this is the actual tree root
    res->tree->lesser_node = res->root_node; // VV_TREE_LESSER must always be used with root reference because of this assignment
}

//
// Create the tree itself. ef is the key evaluation function (or NULL if none), key_type is for default eval function (number comparison)
// sorted is true if there's linked list for fast range access. Returns the tree.
//
vely_tree *vv_tree_create(vely_tree_eval ef, char key_type, bool sorted, bool process)
{
    VV_TRACE("");
    vely_tree *res = vely_calloc (1, sizeof(vely_tree) + (sorted?2*sizeof (vv_tree_node *):0));
    res->process = process;
    res->sorted = sorted; // must be set before vv_tree_node_create() below
    res->eval = ef; // eval function
    res->key_type = key_type;
    // Tree has a dummy node, which has a lesser pointer that points to an actual root of the tree
    // This is so all recursive algorithms work faster without handling exceptions
    res->tree = vv_tree_node_create (sorted?1:0);  //->tree is a dummy node sitting on top of the actual root
    vv_tree_create_root (res, sorted);
    return res;
}

//
// Get the heigh of a node tree. factor is the difference between left and right (if not NULL).
// Node's ->height member is updated by doing this. 
// A node without any children has a height of 1, with at least one child it's 2 etc.
//
inline void vv_tree_height (vv_tree_node *tree, num *factor)
{
    VV_TRACE("");
    num left_height;
    num right_height;
    if (tree->lesser_node == NULL) left_height = 0; else left_height = tree->lesser_node->height;
    if (tree->greater_node == NULL) right_height = 0; else right_height = tree->greater_node->height;
    if (factor) *factor = left_height - right_height; // instead of right-left in classic AVL
    if (left_height > right_height) tree->height = left_height+1; else tree->height = right_height+1;
}


//
// Rotate the node to the right. parent_tree is the parent of the node being rotated, dir is the direction from parent
// to the node being rotated, which is tree.
//
inline void vv_tree_rotate_right (vv_tree_node *parent_tree, int dir, vv_tree_node *tree)
{
    // rotate right
    // save data to use when shuffling below. Old:
    //            T
    //       L         G
    //   LL     LG
    //
    // New:
    //            L 
    //      LL            T
    //                LG     G
    //
    // balance as above
    VV_TREE_SET_PARENT(parent_tree, dir, tree->lesser_node);
    vv_tree_node *t = tree->lesser_node->greater_node;
    tree->lesser_node->greater_node = tree;
    tree->lesser_node = t;
    vv_tree_height (tree, NULL);
    vv_tree_height (parent_tree, NULL);
}

//
// Rotate the node to the left. parent_tree is the parent of the node being rotated, dir is the direction from parent
// to the node being rotated, which is tree.
//
inline void vv_tree_rotate_left (vv_tree_node *parent_tree, int dir, vv_tree_node *tree)
{
    // rotate left 
    // save data to use when shuffling below. Old:
    //            T
    //       L         G
    //            GL     GG
    //
    // New:
    //            G 
    //       T        GG
    //   L      GL
    //
    // balance as above
    VV_TREE_SET_PARENT(parent_tree, dir, tree->greater_node);
    vv_tree_node *t = tree->greater_node->lesser_node;
    tree->greater_node->lesser_node = tree;
    tree->greater_node = t;
    vv_tree_height (tree, NULL);
    vv_tree_height (parent_tree, NULL);
}

//
// Balance 'tree', with parent parent_tree coming to 'tree' by dir direction (left/lesser or right/greater)
//
inline void vv_tree_balance (vv_tree_node *parent_tree, int dir, vv_tree_node *tree)
{
    VV_TRACE("");
    // get the balance factor of the node to balance
    num bal_factor;
    vv_tree_height (tree, &bal_factor);
    //printf("Left %d Right %d\n", tree->lesser_node?tree->lesser_node->height:0, tree->greater_node?tree->greater_node->height:0);
    //assert (-2<=bal_factor && bal_factor<=2);
    
    //Balance factor can be within -2 and 2 inclusive here. The goal is to bring it to 0 or 1/-1.
    if (bal_factor >= 2)
    {
        // This needs balancing to the right, since left is taller. However, if the right subbranch of the left branch is taller than the left subbranch,
        // the right rotation will just make the left-mirror-image of the same problem. Thus, we need to left-rotate the right subbranch of the left branch first.
        if (tree->lesser_node != NULL)
        {
            if ((tree->lesser_node->greater_node ? tree->lesser_node->greater_node->height:0) > (tree->lesser_node->lesser_node?tree->lesser_node->lesser_node->height:0)) vv_tree_rotate_left (tree, VV_TREE_LESSER, tree->lesser_node);
        }
        vv_tree_rotate_right (parent_tree, dir, tree);
        // Recalculate the height of the node rotated, as well as its parent
        vv_tree_height (tree, NULL);
    }
    else if (bal_factor <= -2)
    {
        // This needs balancing to the left, since rigtt is taller. However, if the left subbranch of the right branch is taller than the right subbranch,
        // the leftt rotation will just make the right-mirror-image of the same problem. Thus, we need to right-rotate the left subbranch of the right branch first.
        if (tree->greater_node != NULL)
        {
            if ((tree->greater_node->lesser_node ? tree->greater_node->lesser_node->height:0) > (tree->greater_node->greater_node?tree->greater_node->greater_node->height:0)) vv_tree_rotate_right (tree, VV_TREE_GREATER, tree->greater_node);
        }
        vv_tree_rotate_left (parent_tree, dir, tree);
        // Recalculate the height of the node rotated, as well as its parent
        vv_tree_height (tree, NULL);
    }
    //num bf;
    //vv_tree_height (tree, &bf);
    //assert (bf<=1 && bf>=-1);
}

//
// Insert data with vv_cursor/key/key_len into tree. tree is the node considered, parent_tree/dir is
// its parent and the direction to reach tree (lesser/greater).
// vv_cursor->current/status set.
//
void vv_tree_insert(vv_tree_node *parent_tree, int dir, vv_tree_node *tree, void *data)
{
    VV_TRACE("");
    VV_UNUSED(dir);
    if (tree->key_present  == 0)
    {
        // empty tree node, just add root
        tree->key = vv_cursor->key;
        tree->data = data;
        tree->height = 1;
        tree->key_present = 1;
        if (vv_cursor->root->sorted) 
        {
            // setup a linked list if the first one
            if (!tree->dlist[VV_TREE_LESSER_LIST]) vv_cursor->root->min = tree;
            if (!tree->dlist[VV_TREE_GREATER_LIST]) vv_cursor->root->max = tree;
        }
        vv_cursor->current = tree;
        vv_cursor->status = VV_OKAY;
        vv_cursor->root->count++;
        return;
    }

    // tree has data and possibly subnodes
    int comparison = VV_TREE_EVAL(tree->key);
    if (comparison < 0)
    {
        // this is lesser key
        bool is_new = false;
        if (tree->lesser_node == NULL) 
        {
            // create new one if none there, with no key, that's added in vv_tree_insert below
            tree->lesser_node = vv_tree_node_create(vv_cursor->root->sorted?1:0);
            is_new = true;
            if (vv_cursor->root->sorted) 
            {
                // connect new node into linked list, if this tree has that feature
                if (tree->dlist[VV_TREE_LESSER_LIST]) tree->dlist[VV_TREE_LESSER_LIST]->dlist[VV_TREE_GREATER_LIST] = tree->lesser_node;
                tree->lesser_node->dlist[VV_TREE_LESSER_LIST] = tree->dlist[VV_TREE_LESSER_LIST];
                tree->dlist[VV_TREE_LESSER_LIST] = tree->lesser_node;
                tree->lesser_node->dlist[VV_TREE_GREATER_LIST] = tree;
            }
        }
        vv_tree_insert (tree, VV_TREE_LESSER, tree->lesser_node, data);
        if (!is_new) 
        {
            vv_tree_balance (tree, VV_TREE_LESSER, tree->lesser_node); // don't balance down-node if added a leaf
            vv_tree_balance (parent_tree, dir, tree);
        }
    }
    else if (comparison == 0)
    {
        VELY_ERR0; vv_cursor->status = VV_ERR_EXIST; 
        return;
    }
    else
    {
        // this is greater key
        bool is_new = false;
        if (tree->greater_node == NULL) 
        {
            // create new one if none there, with no key, that's added in vv_tree_insert below
            tree->greater_node = vv_tree_node_create(vv_cursor->root->sorted?1:0);
            is_new = true;
            if (vv_cursor->root->sorted)
            {
                // connect new node into linked list, if this tree has that feature
                if (tree->dlist[VV_TREE_GREATER_LIST]) tree->dlist[VV_TREE_GREATER_LIST]->dlist[VV_TREE_LESSER_LIST] = tree->greater_node;
                tree->greater_node->dlist[VV_TREE_GREATER_LIST] = tree->dlist[VV_TREE_GREATER_LIST];
                tree->dlist[VV_TREE_GREATER_LIST] = tree->greater_node;
                tree->greater_node->dlist[VV_TREE_LESSER_LIST] = tree;
            }
        }
        vv_tree_insert (tree, VV_TREE_GREATER, tree->greater_node, data);
        if (!is_new) 
        {
            vv_tree_balance (tree, VV_TREE_GREATER, tree->greater_node); // don't balance down-node if added a leaf
            vv_tree_balance (parent_tree, dir, tree);
        }
    }
    //VV_UNUSED(parent_tree);
    /*char lh = tree->lesser_node?tree->lesser_node->height:0;
    char rh = tree->greater_node?tree->greater_node->height:0;
    int f = (lh-rh);
    if (f<0) f=-f;
    assert(f<2);*/
}


//
// Find lesser or equal key to that of vv_cursor->key. If 'equal' is true, then search for equal as well.
// vv_cursor->current/status set.
//
void vv_tree_search_lesser_equal (vv_tree_node *tree, bool equal)
{
    VV_TRACE("");
    vv_tree_node *prev_lesser = NULL;
    // Start from node 'tree' which is usually given as top root
    // go down the tree until found, if there's no key (empty tree), just declare not found below since prev_lesser is NULL
    if (tree && tree->key_present != 0) { 
        while (tree)
        {
            // check if key lesser, equal, greater
            int cmp = VV_TREE_EVAL(tree->key);
            if (cmp == 0)
            {
                // if equal not requested, then go the lesser node (since this is lesser search)
                if (!equal) { cmp = -1; } 
                else
                {
                    // and if equal requested, done
                    vv_cursor->status = VV_OKAY;
                    vv_cursor->current = tree;
                    return;
                }
            } 
            // check if key lesser or equal, move down the tree, cannot be 'else' here
            // since we set cmp manually in one case above
            if (cmp < 0) { tree = tree->lesser_node;  }
            else 
            { 
                prev_lesser = tree;  
                tree = tree->greater_node; 
            }
        }
    }
    // Here we come when we exhausted the search and are at some node where key is either lesser or greater
    // than the previous node, but in that direction there's nothing (NULL child)
    if (prev_lesser != NULL)
    {
        // this is the last key that was lesser (i.e. key search for was greater), and that's the maximum lesser key
        vv_cursor->status = VV_OKAY;
        vv_cursor->current = prev_lesser;
        return;
    }
    else 
    {
        // there was no lesser key, so nothing
        VELY_ERR0; vv_cursor->status = VV_ERR_EXIST;
    }
    return;
}

//
// Find greater or equal key to that of vv_cursor->key. If 'equal' is true, then search for equal as well.
// vv_cursor->current/status set.
//
void vv_tree_search_greater_equal (vv_tree_node *tree, bool equal)
{
    VV_TRACE("");
    vv_tree_node *prev_greater = NULL;
    // start from the top
    // go down the tree until found, if there's no key (empty tree), just declare not found below since prev_greater is NULL
    if (tree && tree->key_present != 0) {
        while (tree)
        {
            int cmp = VV_TREE_EVAL(tree->key);
            if (cmp == 0)
            {
                if (!equal) { cmp = 1; } // if equal not requested, go down greater path
                else
                {
                    vv_cursor->status = VV_OKAY; // if equal, and 'equal' requested
                    vv_cursor->current = tree;
                    return;
                }
            } 
            // since we manipulate cmp above, cannot do 'else' here. Move down the tree 
            if (cmp < 0) 
            {
                prev_greater = tree;  
                tree = tree->lesser_node; 
            }
            else { tree = tree->greater_node; }
        }
    }
    if (prev_greater != NULL)
    {
        // the last key found greater than requested, is the minimum greater one
        vv_cursor->status = VV_OKAY;
        vv_cursor->current = prev_greater;
        return;
    }
    else 
    {
        // no key found greater
        VELY_ERR0; vv_cursor->status = VV_ERR_EXIST;
    }
    return;
}


// 
// Search for the exact vv_cursor->key 
// vv_cursor->current/status set.
//
void vv_tree_search (vv_tree_node *tree)
{
    VV_TRACE("");
    // go down the tree until found, if there's no key (empty tree), just declare not found below
    if (tree && tree->key_present != 0) { 
        while (tree) 
        {
            int cmp = VV_TREE_EVAL(tree->key);
            if (cmp == 0)
            {
                vv_cursor->status = VV_OKAY;
                vv_cursor->current = tree;
                return;
            }
            if (cmp < 0) tree = tree->lesser_node; 
            else tree = tree->greater_node;
        }
    }
    // if here, none found
    VELY_ERR0; vv_cursor->status = VV_ERR_EXIST;
    return;
}


//
// Search for minimum key. lcurs is the cursor to set, orig_tree is the root of the tree
// vv_cursor->current/status set.
//
void vv_tree_min_f (vely_tree_cursor *lcurs, vely_tree *orig_tree)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
    if (orig_tree->sorted)
    {
        // If there has a linked list, we have it's head on the left right away
        if (orig_tree->count == 0) { VELY_ERR0; vv_cursor->status = VV_ERR_EXIST; return;}
        vv_cursor->status = VV_OKAY;
        vv_cursor->current = orig_tree->min;
    }
    else
    {
        // if we don't have a linked list, go down the tree, getting lesser and lesser 
        // until nothing found. The last one is the smallest.
        vv_tree_node *cur = orig_tree->tree->lesser_node;
        //this takes care of empty tree
        if (orig_tree->count == 0) { VELY_ERR0; vv_cursor->status = VV_ERR_EXIST; return;}
        vv_cursor->status = VV_OKAY;
        while (cur->lesser_node) cur = cur->lesser_node;
        vv_cursor->current = cur;
    }
}

//
// Search for maximum key. lcurs is the cursor to set, orig_tree is the root of the tree
// vv_cursor->current/status set.
//
void vv_tree_max_f (vely_tree_cursor *lcurs, vely_tree *orig_tree)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
    if (orig_tree->sorted)
    {
        // if linked list present, get the max, i.e. head on the right
        if (orig_tree->count == 0) { VELY_ERR0; vv_cursor->status = VV_ERR_EXIST; return;}
        vv_cursor->status = VV_OKAY;
        vv_cursor->current = orig_tree->max;
    }
    else
    {
        // if no linked list, go down greater always, until no greater found
        vv_tree_node *cur = orig_tree->tree->lesser_node;
        //this takes care of empty tree
        if (orig_tree->count == 0) { VELY_ERR0; vv_cursor->status = VV_ERR_EXIST; return;}
        vv_cursor->status = VV_OKAY;
        while (cur->greater_node) cur = cur->greater_node;
        vv_cursor->current = cur;
    }
}

//
// Part of deleting a node, which is the most complex operation here. Deleting a node (when there's a greater branch from it)
// works by finding the lowest key in the greater node and copying it to node being deleted, then deleting this lowest key node, which is
// easy since it's always a leaf. 'tree_greater_node' is being looked at and we arrived to it from parent tree going in 'dir' direction.
// found is the actual node with found key (vv_cursor->key)
//
void vv_tree_find_leaf_del (vv_tree_node *parent_tree, int dir, vv_tree_node *tree_greater_node, vv_tree_node *found)
{
    VV_TRACE("");
    // Here we go to the lowest key in this branch of the tree
    if (tree_greater_node->lesser_node == NULL)
    {
        // once no more lesser nodes, this is the node to copy in place of found and then to be deleted
        if (vv_cursor->root->sorted)
        {
            // if linked list, set it up
            if (found->dlist[VV_TREE_LESSER_LIST]) found->dlist[VV_TREE_LESSER_LIST]->dlist[VV_TREE_GREATER_LIST] = found->dlist[VV_TREE_GREATER_LIST]; else vv_cursor->root->min = found->dlist[VV_TREE_GREATER_LIST];
            if (found->dlist[VV_TREE_GREATER_LIST]) found->dlist[VV_TREE_GREATER_LIST]->dlist[VV_TREE_LESSER_LIST] = found->dlist[VV_TREE_LESSER_LIST]; else vv_cursor->root->max = found->dlist[VV_TREE_LESSER_LIST];
            found->dlist[VV_TREE_LESSER_LIST] = tree_greater_node->dlist[VV_TREE_LESSER_LIST];
            found->dlist[VV_TREE_GREATER_LIST] = tree_greater_node->dlist[VV_TREE_GREATER_LIST];
            if (tree_greater_node->dlist[VV_TREE_LESSER_LIST]) tree_greater_node->dlist[VV_TREE_LESSER_LIST]->dlist[VV_TREE_GREATER_LIST] = found; else vv_cursor->root->min = found;
            if (tree_greater_node->dlist[VV_TREE_GREATER_LIST]) tree_greater_node->dlist[VV_TREE_GREATER_LIST]->dlist[VV_TREE_LESSER_LIST] = found; else vv_cursor->root->max = found;
        }
        // put this leaf node's key/data into one to delete
        found->key = tree_greater_node->key;
        found->data = tree_greater_node->data;
        // make sure leaf node's parent is connected property
        VV_TREE_SET_PARENT(parent_tree, dir, tree_greater_node->greater_node);
        vv_tree_node_delete (tree_greater_node); // delete leaf node
        // no need to balance previous tree_greater_node->greater_node because it can be only 1 extra in height
        // and parent_tree is balanced in caller
        return;
    }
    else 
    {
        // go down lesser until above found; each time we back up, balance the lesser part (which is at the bottom the parent of the 
        // node found above (lesser_node == NULL)
        vv_tree_find_leaf_del (tree_greater_node, VV_TREE_LESSER, tree_greater_node->lesser_node, found);
        if (tree_greater_node->lesser_node) vv_tree_balance (tree_greater_node, VV_TREE_LESSER, tree_greater_node->lesser_node);
        vv_tree_balance (parent_tree, dir, tree_greater_node); // then balance it's parent tree_greater_node
        return;
    }
    return; 
}

//
// Delete a node with vv_cursor->key key. tree is the node looked at, and we arrived at it by going in
// 'dir' direction from parent_tree (so dir is either lesser or greater).
//
void vv_tree_delete (vv_tree_node *parent_tree,  int dir, vv_tree_node *tree)
{
    VV_TRACE("");
    void *res = NULL;
    char *rkey;
    // compare fixed key with tree->key 
    int cmp = VV_TREE_EVAL(tree->key);
    if (cmp == 0)
    {
        // if equal, save pointers to data and key before proceeding to delete.
        res = tree->data;
        rkey = tree->key;
        if (tree->greater_node == NULL)
        {
            // if there is no greater node, then connect parent lesser node with the deleting-node's lesser one, easy case
            VV_TREE_SET_PARENT(parent_tree, dir, tree->lesser_node);
            if (vv_cursor->root->sorted)
            {
                // update the linked list. dlist may or may not be here (meaning allocated); it's not if sorted is false.
                if (tree->dlist[VV_TREE_LESSER_LIST]) tree->dlist[VV_TREE_LESSER_LIST]->dlist[VV_TREE_GREATER_LIST] = tree->dlist[VV_TREE_GREATER_LIST]; else vv_cursor->root->min = tree->dlist[VV_TREE_GREATER_LIST];
                if (tree->dlist[VV_TREE_GREATER_LIST]) tree->dlist[VV_TREE_GREATER_LIST]->dlist[VV_TREE_LESSER_LIST] = tree->dlist[VV_TREE_LESSER_LIST]; else vv_cursor->root->max = tree->dlist[VV_TREE_LESSER_LIST];
            }
            // delete the node, free it up
            vv_tree_node_delete (tree);
        }
        else
        {
            // if there is a greater node, go down to find the lowest key node in the 'greater' branch from the node to be deleted.
            // This lowest key node is always leaf and once the node to be deleted is taken out, it can take its place in the tree.
            // Then we move that node's key and data to the one we're 'deleting', and actually delete the leaf one.
            vv_tree_find_leaf_del (tree, VV_TREE_GREATER, tree->greater_node, tree); // the above is done here.
            // balance the node we started down towards, and then it's parent; they all may be affected
            if (tree->greater_node) vv_tree_balance (tree, VV_TREE_GREATER, tree->greater_node);
            vv_tree_balance (parent_tree, dir, tree);
        }
        // setup result
        vv_cursor->status = VV_OKAY;
        vv_cursor->root->count--;
        vv_cursor->res = res;
        vv_cursor->rkey = rkey;
        return;
    }
    else
    {
        // go down the tree until exhausted; if nothing found, nothing to delete
        if (cmp < 0 && tree->lesser_node) 
        {
            vv_tree_delete (tree, VV_TREE_LESSER, tree->lesser_node);
            // balance both the tree we went down towards and its parent
            if (tree->lesser_node != NULL) vv_tree_balance (tree, VV_TREE_LESSER, tree->lesser_node);
            vv_tree_balance (parent_tree, dir, tree);
            return;
        }
        if (cmp > 0 && tree->greater_node)
        {
            vv_tree_delete (tree, VV_TREE_GREATER, tree->greater_node);
            // balance both the tree we went down towards and its parent
            if (tree->greater_node) vv_tree_balance (tree, VV_TREE_GREATER, tree->greater_node);
            vv_tree_balance (parent_tree, dir, tree);
            return;
        }
    }
    // nothing to delete is here
    VELY_ERR0; vv_cursor->status = VV_ERR_EXIST;
    return;
}

//
// Internal support. Check the whole tree to make sure it's perfectly balanced. Used it tests to prove the tree
// is always perfectly balanced no matter what. It simply traverses the whole tree and checks each and every node.
// Returns >0 if problem.
//
num vv_tree_bal (vv_tree_node *tree)
{
    VV_TRACE("");
    num res = 0;
    if (tree->lesser_node) res += vv_tree_bal(tree->lesser_node);
    if (tree->greater_node) res += vv_tree_bal (tree->greater_node); 

    int f = (tree->lesser_node?tree->lesser_node->height:0) - (tree->greater_node?tree->greater_node->height:0);
    if (f < -1 || f > 1) {
        printf("VELERROR %d %s %s\n", f, tree->lesser_node==NULL?"lesser null":"", tree->greater_node==NULL ? "greater null":"");
        return 1+res; 
    } else return res;
}


//
// Top level API for purge. 
//
void vv_tree_purge_f (vely_tree *orig_tree)
{
    VV_TRACE("");
    if (orig_tree->count != 0) vely_report_error ("Cannot purge non-empty tree. Delete all nodes first.");
    if (orig_tree->tree->lesser_node != NULL) vv_tree_node_delete (orig_tree->tree->lesser_node); 
    vely_free (orig_tree->tree); 
    vely_free(orig_tree);
}

//
// Top level API for search. lcurs is the cursor, orig_tree is the tree structure, key/key_len to search for.
// Sets vv_cursor->current to found, or ->status to VV_ERR_EXIST if not found (otherwise VV_OKAY). 
// If key_len is -1, sets key_len to strlen() of key
//
void vv_tree_search_f (vely_tree_cursor *lcurs, vely_tree *orig_tree, char *key, num key_len)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
#ifdef DEBUG
    vv_cursor->root->hops=0;
#endif
    if (key_len == -1) vv_cursor->key_len = strlen (key); else vv_cursor->key_len = key_len;
    vv_cursor->key = key; 
    vv_tree_search (orig_tree->tree->lesser_node);
}

//
// Top level API for delete. lcurs is the cursor, orig_tree is the tree structure, key/key_len to delete.
// Sets vv_cursor->rkey/res as key/data of deleted node. If key_len is -1, sets key_len to strlen() of key
//
void vv_tree_delete_f (vely_tree_cursor *lcurs, vely_tree *orig_tree, char *key, num key_len)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
#ifdef DEBUG
    vv_cursor->root->hops=0;
#endif
    if (key_len == -1) vv_cursor->key_len = strlen (key); else vv_cursor->key_len = key_len;
    vv_cursor->key = key; 
    if (orig_tree->tree->lesser_node && orig_tree->tree->lesser_node->key_present != 0) 
    {
        vv_tree_delete (orig_tree->tree, VV_TREE_LESSER, orig_tree->tree->lesser_node);
        // check if root deleted. If so, create empty root (with no key), or otherwise nothing else will work on the tree.
        if (orig_tree->tree->lesser_node == NULL) vv_tree_create_root (orig_tree, orig_tree->sorted);
    }
    else 
    {
        VELY_ERR0; vv_cursor->status = VV_ERR_EXIST;
    }
}

//
// Top level API for insert. lcurs is the cursor, orig_tree is the tree structure, key/key_len to delete.
// Sets vv_cursor->cursor to inserted. If key_len is -1, sets key_len to strlen() of key
//
void vv_tree_insert_f (vely_tree_cursor *lcurs, vely_tree *orig_tree, char *key, num key_len, void *data)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
#ifdef DEBUG
        vv_cursor->root->hops=0;
#endif
    if (key_len == -1) vv_cursor->key_len = strlen (key); else vv_cursor->key_len = key_len;
    vv_cursor->key = key;
    // h->process is the same as vely_mem_process (set in v1.c to be equal)
    if (orig_tree->process && !vely_mem_process_key) _vely_mem_set_process (key);
    if (orig_tree->process && !vely_mem_process_data) _vely_mem_set_process (data);
    vv_tree_insert (orig_tree->tree, VV_TREE_LESSER, orig_tree->tree->lesser_node ,data);
}
 
//
// Top level API for search <=. lcurs is the cursor, orig_tree is the tree structure, equal is true if it's <= otherwise <
// key/key_len to seach for.
// Sets vv_cursor->current to found, or ->status to VV_ERR_EXIST if not found (otherwise VV_OKAY). 
//
void vv_tree_search_lesser_equal_f (vely_tree_cursor *lcurs, vely_tree *orig_tree, bool equal, char *key, num key_len)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
#ifdef DEBUG
    vv_cursor->root->hops=0;
#endif
    if (key_len == -1) vv_cursor->key_len = strlen (key); else vv_cursor->key_len = key_len;
    vv_cursor->key = key; 
    vv_tree_search_lesser_equal (orig_tree->tree->lesser_node, equal);
}

//
// Top level API for search >=. lcurs is the cursor, orig_tree is the tree structure, equal is true if it's >= otherwise >
// key/key_len to seach for.
// Sets vv_cursor->current to found, or ->status to VV_ERR_EXIST if not found (otherwise VV_OKAY). 
//
void vv_tree_search_greater_equal_f (vely_tree_cursor *lcurs, vely_tree *orig_tree, bool equal, char *key, num key_len)
{
    VV_TRACE("");
    vv_cursor = lcurs;
    vv_cursor->root = orig_tree;
#ifdef DEBUG
        vv_cursor->root->hops=0;
#endif
    if (key_len == -1) vv_cursor->key_len = strlen (key); else vv_cursor->key_len = key_len;
    vv_cursor->key = key;
    vv_tree_search_greater_equal (orig_tree->tree->lesser_node, equal);
}

