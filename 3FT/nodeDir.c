/*--------------------------------------------------------------------*/
/* nodeDir.c                                                          */
/* Author: Anton Stengel and Jake Intrater                            */
/*--------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#include "nodeDir.h"
#include "dynarray.h"


/* A node structure representing a dir. */
struct nodeDir {
   /* the full path of this directory */
   char* path;

   /* the parent directory of this node
      NULL for the root of the directory tree */
   NodeDir parent;

   /* the subdirectories of this directory
      stored in sorted order by pathname */
   DynArray_T childrenDirs;

   /* the subfiles of this directory
      stored in sorted order by pathname */
   DynArray_T childrenFiles;
};


/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owened by the caller!
*/
static char* NodeDir_buildPath(NodeDir n, const char* dir) {
   char* path;

   assert(dir != NULL);

   if(n == NULL)
      path = malloc(strlen(dir)+1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}

/* see nodeDir.h for specification */
NodeDir NodeDir_create(const char* name, NodeDir parent) {

   NodeDir new;

   assert(name != NULL);

   new = malloc(sizeof(struct nodeDir));
   if(new == NULL)
      return NULL;

   new->path = NodeDir_buildPath(parent, name);

   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;

   new->childrenDirs = DynArray_new(0);
   if(new->childrenDirs == NULL) {
      free(new->path);
      free(new);
      return NULL;
   }
   new->childrenFiles = DynArray_new(0);
   if(new->childrenFiles == NULL) {
      free(new->path);
      free(new);
      return NULL;
   }

   return new;
}


/* see nodeDir.h for specification */
size_t NodeDir_destroy(NodeDir n) {
    size_t i;
    size_t count = 0;
    NodeDir c;

    assert(n != NULL);

    /* free all children files of n */
    for (i = 0; i < DynArray_getLength(n->childrenFiles); i++) {
        (void) NodeFile_destroy(DynArray_get(n->childrenFiles, i));
    }
    DynArray_free(n->childrenFiles);

    /* recursively call destroy on each child dir */
    for (i = 0; i < DynArray_getLength(n->childrenDirs); i++) {
        c = DynArray_get(n->childrenDirs, i);
        count += NodeDir_destroy(c);
    }
    DynArray_free(n->childrenDirs);

    free(n->path);
    free(n);
    count++;

    return count;
}


/* see nodeDir.h for specification */
int NodeDir_compare(NodeDir node1, NodeDir node2) {
    assert(node1 != NULL);
    assert(node2 != NULL);

    return strcmp(node1->path, node2->path);
}


/* see nodeDir.h for specification */
const char* NodeDir_getPath(NodeDir n) {
    assert(n != NULL);
    return n->path;
}


/* see nodeDir.h for specification */
size_t NodeDir_getNumChildDirs(NodeDir n) {
    assert(n != NULL);
    return DynArray_getLength(n->childrenDirs);
}


/* see nodeDir.h for specification */
size_t NodeDir_getNumChildFiles(NodeDir n) {
    assert(n != NULL);
    return DynArray_getLength(n->childrenFiles);
}


/* see nodeDir.h for specification */
int NodeDir_hasChildDir(NodeDir n, const char* path, size_t* 
childIndex) {
    size_t index;
    int result;
    NodeDir checker;

    assert(n != NULL);
    assert(path != NULL);

    checker = NodeDir_create(path, NULL);
    if(checker == NULL)
        return -1;
    result = DynArray_bsearch(n->childrenDirs, checker, &index,
                (int (*)(const void*, const void*)) NodeDir_compare);
    (void) NodeDir_destroy(checker);

    if(childIndex != NULL)
        *childIndex = index;
    return result;
}


/* see nodeDir.h for specification */
int NodeDir_hasChildFile(NodeDir n, const char* path, 
size_t* childIndex) {
    size_t index;
    int result;
    NodeFile checker;

    assert(n != NULL);
    assert(path != NULL);

    checker = NodeFile_create(path, NULL, NULL, (size_t)0);
    if(checker == NULL)
        return -1;
    result = DynArray_bsearch(n->childrenFiles, checker, &index,
                (int (*)(const void*, const void*)) NodeFile_compare);
    (void) NodeFile_destroy(checker);

    if(childIndex != NULL)
        *childIndex = index;
    return result;
}


/* see nodeDir.h for specification */
NodeDir NodeDir_getChildDir(NodeDir n, size_t childIndex) {
    assert(n != NULL);

    if (DynArray_getLength(n->childrenDirs) > childIndex)
        return DynArray_get(n->childrenDirs, childIndex);
    else
        return NULL;
}


/* see nodeDir.h for specification */
NodeFile NodeDir_getChildFile(NodeDir n, size_t childIndex) {
    assert(n != NULL);

    if (DynArray_getLength(n->childrenFiles) > childIndex)
        return DynArray_get(n->childrenFiles, childIndex);
    else
        return NULL;
}


/* see nodeDir.h for specification */
NodeDir NodeDir_getParent(NodeDir n) {
    assert(n != NULL);
    return n->parent;
}


/* see nodeDir.h for specification */
int NodeDir_linkChildDir(NodeDir parent, NodeDir child) {
    size_t i;
    char* rest;

    assert(parent != NULL);
    assert(child != NULL);

    /* checks for different kinds of errors */
    if (NodeDir_hasChildDir(parent, child->path, NULL))
        return ALREADY_IN_TREE;
    if (NodeDir_hasChildFile(parent, child->path, NULL))
        return ALREADY_IN_TREE;

    i = strlen(parent->path);
    if (strncmp(child->path, parent->path, i))
        return PARENT_CHILD_ERROR;
    rest = child->path + i;
    if (strlen(child->path) >= i && rest[0] != '/')
        return PARENT_CHILD_ERROR;
    rest++;
    if (strstr(rest, "/") != NULL)
        return PARENT_CHILD_ERROR;

    child->parent = parent;

    /* checks if parent already has a child with child's path */
    if (DynArray_bsearch(parent->childrenDirs, child, &i,
            (int (*)(const void*, const void*)) NodeDir_compare) == 1)
        return ALREADY_IN_TREE;

    if (DynArray_addAt(parent->childrenDirs, i, child) == TRUE)
        return SUCCESS;
    else
        return PARENT_CHILD_ERROR;
}


/* see nodeDir.h for specification */
int NodeDir_linkChildFile(NodeDir parent, NodeFile child) {
    size_t i;
    char* rest;

    assert(parent != NULL);
    assert(child != NULL);

    /* checks for different kinds of errors */
    if (NodeDir_hasChildDir(parent, NodeFile_getPath(child), NULL))
        return ALREADY_IN_TREE;
    if (NodeDir_hasChildFile(parent, NodeFile_getPath(child), NULL))
        return ALREADY_IN_TREE;

    i = strlen(parent->path);
    if (strncmp(NodeFile_getPath(child), parent->path, i))
        return PARENT_CHILD_ERROR;
    rest = (char*) NodeFile_getPath(child) + i;
    if (strlen(NodeFile_getPath(child)) >= i && rest[0] != '/')
        return PARENT_CHILD_ERROR;
    rest++;
    if (strstr(rest, "/") != NULL)
        return PARENT_CHILD_ERROR;

    /* checks if parent already has a child with child's path */
    if (DynArray_bsearch(parent->childrenFiles, child, &i,
            (int (*)(const void*, const void*)) NodeFile_compare) == 1)
        return ALREADY_IN_TREE;

    if (DynArray_addAt(parent->childrenFiles, i, child) == TRUE)
        return SUCCESS;
    else
        return PARENT_CHILD_ERROR;
}


/* see nodeDir.h for specification */
int NodeDir_unlinkChildDir(NodeDir parent, NodeDir child) {
    size_t i;

    assert(parent != NULL);
    assert(child != NULL);

    if(DynArray_bsearch(parent->childrenDirs, child, &i,
            (int (*)(const void*, const void*)) NodeDir_compare) == 0)
        return PARENT_CHILD_ERROR;

    (void) DynArray_removeAt(parent->childrenDirs, i);
    return SUCCESS;
}


/* see nodeDir.h for specification */
int NodeDir_unlinkChildFile(NodeDir parent, NodeFile child) {
    size_t i;

    assert(parent != NULL);
    assert(child != NULL);

    if(DynArray_bsearch(parent->childrenFiles, child, &i,
            (int (*)(const void*, const void*)) NodeFile_compare) == 0)
        return PARENT_CHILD_ERROR;

    (void) DynArray_removeAt(parent->childrenFiles, i);
    return SUCCESS;
}