/*--------------------------------------------------------------------*/
/* nodeDir.h                                                          */
/* Author: Anton Stengel (and Jake Intrater)                          */
/*--------------------------------------------------------------------*/


#ifndef NODEDIR_INCLUDED
#define NODEDIR_INCLUDED


#include <stddef.h>
#include "a4def.h"


/*
    a NodeDir is a node that contains its path, a referenec to its
    parent node, and children nodes (both NodeDirs and NodeFiles).
*/
typedef struct nodeDir* NodeDir;


#include "nodeFile.h"


/*
    Creates and returns a new NodeDir or NULL if allocation error
    occurs. NodeDir's path is parent's path (if it exists) prefixed
    to "name" separated by a slash. It points to its parent.

    Note: the parent is not linked to this new NodeDir.
*/
NodeDir NodeDir_create(const char* name, NodeDir parent); 


/*
    Destroys the entire hierarchy of Nodes rooted at NodeDir n,
    including n itself. Returns the number of NodeDirs destroyed.
*/
size_t NodeDir_destroy(NodeDir n);


/*
    Compares node1 and node2 based on their paths.
    Returns <0, 0, or >0 if node1 is less than,
    equal to, or greater than node2, respectively.
*/
int NodeDir_compare(NodeDir node1, NodeDir node2);


/*
    Returns NodeDir n's path.
*/
const char* NodeDir_getPath(NodeDir n);


/*
    Returns the number of child NodeDirs n has.
*/
size_t NodeDir_getNumChildDirs(NodeDir n);


/*
    Returns the number of child NodeFiles n has.
*/
size_t NodeDir_getNumChildFiles(NodeDir n);


/*
    Returns 1 if NodeDir n has a child NodeDir with path,
    0 if not, and -1 if allocation error. Passes index of child 
    back with childIndex.
*/
int NodeDir_hasChildDir(NodeDir n, const char* path, size_t* 
childIndex);


/*
    Returns 1 if n has a child NodeFile with path,
    0 if not, and -1 if allocation error. Passes index of child 
    back with childIndex.
*/
int NodeDir_hasChildFile(NodeDir n, const char* path, size_t* 
childIndex);


/*
    Returns the child NodeDir of n with index childIndex
    or NULL if it doesn't exist.
*/
NodeDir NodeDir_getChildDir(NodeDir n, size_t childIndex);


/*
    Returns the child NodeFile of n with index childIndex
    or NULL if it doesn't exist.
*/
NodeFile NodeDir_getChildFile(NodeDir n, size_t childIndex);


/*
    Returns the parent of NodeDir n or NULL if it does not exist.
*/
NodeDir NodeDir_getParent(NodeDir n);


/*
    Makes NodeDir child a child of parent and returns SUCCESS.
    This is not possible in the following cases:
    * child's path is not parent's path + / + directory,
      in which case returns PARENT_CHILD_ERROR
    * parent already has a child with child's path,
      in which case returns ALREADY_IN_TREE
    * parent is unable to allocate memory to store new child link,
      in which case returns MEMORY_ERROR
*/
int NodeDir_linkChildDir(NodeDir parent, NodeDir child);


/*
    Makes NodeFile child a child of parent and returns SUCCESS.
    This is not possible in the following cases:
    * child's path is not parent's path + / + directory,
      in which case returns PARENT_CHILD_ERROR
    * parent already has a child with child's path,
      in which case returns ALREADY_IN_TREE
    * parent is unable to allocate memory to store new child link,
      in which case returns MEMORY_ERROR
*/
int NodeDir_linkChildFile(NodeDir parent, NodeFile child);


/*
    Unlinks NodeDir parent from its NodeDir child, leaving
    child unchanged.

    Returns PARENT_CHILD_ERROR if child is not a child of parent,
    and SUCCESS otherwise.
*/
int NodeDir_unlinkChildDir(NodeDir parent, NodeDir child);


/*
    Unlinks NodeDir parent from its NodeFile child, leaving
    child unchanged.

    Returns PARENT_CHILD_ERROR if child is not a child of parent,
    and SUCCESS otherwise.
*/
int NodeDir_unlinkChildFile(NodeDir parent, NodeFile child);

#endif