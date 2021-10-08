/*--------------------------------------------------------------------*/
/* nodeFile.c                                                         */
/* Author: Anton Stengel and Jake Intrater                            */
/*--------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#include "nodeFile.h"
#include "dynarray.h"


/* A node structure representing a file. */
struct nodeFile {
   /* the full path of this directory */
   char* path;

   /* the parent directory of this node
      NULL for the root of the directory tree */
   NodeDir parent;

   /* void pointer to contents stored in file */
   void *contents;

   /* size_t length of contents */
   size_t length;
};


/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owened by the caller.
*/
static char* NodeFile_buildPath(NodeDir n, const char* dir) {
   char* path;

   assert(dir != NULL);

   if(n == NULL)
      path = malloc(strlen(dir)+1);
   else
      path = malloc(strlen(NodeDir_getPath(n)) + 1 + strlen(dir) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, NodeDir_getPath(n));
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}


/* See nodeFile.h for specification. */
NodeFile NodeFile_create(const char* name, NodeDir parent, 
void* contents, size_t length) {
   NodeFile new;

   assert(name != NULL);

   new = malloc(sizeof(struct nodeFile));
   if(new == NULL)
      return NULL;

   new->path = NodeFile_buildPath(parent, name);

   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;
   new->contents = contents;
   new->length = length;

   return new;
}


/* See nodeFile.h for specification. */
size_t NodeFile_destroy(NodeFile n) {
    assert(n != NULL);
    
    free(n->path);
    free(n);

    return 1;
}


/* See nodeFile.h for specification. */
int NodeFile_compare(NodeFile node1, NodeFile node2) {
    assert(node1 != NULL);
    assert(node2 != NULL);

    return strcmp(node1->path, node2->path);
}


/* See nodeFile.h for specification. */
const char* NodeFile_getPath(NodeFile n) {
    assert(n != NULL);
    return n->path;
}


/* See nodeFile.h for specification. */
NodeDir NodeFile_getParent(NodeFile n) {
    assert(n != NULL);
    return n->parent;
}


/* See nodeFile.h for specification. */
void *NodeFile_getContents(NodeFile n) {
    assert(n != NULL);
    return n->contents;
}


/* See nodeFile.h for specification. */
void *NodeFile_replaceContents(NodeFile n, void *newContents, 
size_t newLength) {
    void *oldContents;

    assert(n != NULL);

    oldContents = n->contents;
    n->contents = newContents;
    n->length = newLength;

    return oldContents;
}


/* See nodeFile.h for specification. */
size_t NodeFile_getLength(NodeFile n) {
    assert(n != NULL);
    return n->length;
}