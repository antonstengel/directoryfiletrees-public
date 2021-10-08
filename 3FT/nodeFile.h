/*--------------------------------------------------------------------*/
/* nodeFile.h                                                         */
/* Author: Anton Stengel and Jake Intrater                            */
/*--------------------------------------------------------------------*/


#ifndef NODEFILE_INCLUDED
#define NODEFILE_INCLUDED


#include <stddef.h>
#include "a4def.h"


/*
    a NodeFile is a node that contains its path, a referenec to its
    parent node, a pointer to its contents, and its contents' length.
*/
typedef struct nodeFile* NodeFile;


#include "nodeDir.h"


/*
    Creates and returns a new NodeFile or NULL if allocation error
    occurs. NodeFile's path is parent's path (if it exists) prefixed
    to "name" separated by a slash. It points to its parent. Also
    adds contents and length to NodeFile. Note that client still owns
    contents.

    Note: the parent is not linked to this new NodeFile
*/
NodeFile NodeFile_create(const char* name, NodeDir parent, 
void* contents, size_t length); 


/*
    Destroys NodeFile n. Returns the number of NodeDir's destroyed,
    which is always 0.
*/
size_t NodeFile_destroy(NodeFile n);


/*
    Compares node1 and node2 based on their paths.
    Returns <0, 0, or >0 if node1 is less than,
    equal to, or greater than node2, respectively.
*/
int NodeFile_compare(NodeFile node1, NodeFile node2);


/*
    Returns NodeFile n's path.
*/
const char* NodeFile_getPath(NodeFile n);


/*
    Returns the the parent NodeDir of NodeFile n.
*/
NodeDir NodeFile_getParent(NodeFile n);


/*
    Returns the contents of NodeFile n.
*/
void* NodeFile_getContents(NodeFile n);


/*
    Replaces NodeFile n's contents and length with newContents and
    newLength and returns the old contents.
*/
void *NodeFile_replaceContents(NodeFile n, void *newContents, 
size_t newLength);


/*
    Returns the length of NodeFile n.
*/
size_t NodeFile_getLength(NodeFile n);

#endif