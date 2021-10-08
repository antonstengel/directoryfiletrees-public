/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Anton Stengel and Jake Intrater                            */
/*--------------------------------------------------------------------*/


/* CHECK THAT WE NEED ALL THESE BEFORE SUBMITTING */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>


#include "dynarray.h"
#include "ft.h"
#include "nodeDir.h" /* this includes nodeFile.h too */


/**********************************************************************/


/* A Directory Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;

/* (only one of these will ever be non-NULL) */
/* a pointer to the root NodeDir in the hierarchy */
static NodeDir rootDir;
/* a pointer to the root NodeFile in the hierarchy */
static NodeFile rootFile;

/* a counter for the number of NodeDirs in the hierarchy */
static size_t countDirs;


/**********************************************************************/
/* Simple static helper functions */
/**********************************************************************/


/*
   Destroys the entire hierarchy of Nodes rooted at NodeDir curr,
   including curr itself.
*/
static void FT_removePathFromDir(NodeDir curr) {
   if(curr != NULL) {
      countDirs -= NodeDir_destroy(curr);
   }
}


/*
   Given a prospective parent NodeDir and child NodeDir,
   adds child to parent's children list, if possible.

   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChildDir(NodeDir parent, NodeDir child) {

   assert(parent != NULL);

   if(NodeDir_linkChildDir(parent, child) != SUCCESS) {
      (void) NodeDir_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}


/*
   Given a prospective parent NodeDir and child NodeFile,
   adds child to parent's children list, if possible

   If not possible, destroys the child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChildFile(NodeDir parent, NodeFile child) {

   assert(parent != NULL);

   if(NodeDir_linkChildFile(parent, child) != SUCCESS) {
      (void) NodeFile_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}


/**********************************************************************/
/* Static functions for traversing path */
/**********************************************************************/


/*
    Helper recursive function for FT_traversePathDir.

    Starting at the parameter curr, traverses as far down
    the hierarchy as possible, just considering NodeDir's, while
    still matching the path parameter.

    Returns a pointer to the farthest matching NodeDir down that path,
    or NULL if there is no NodeDir in curr's hierarchy that matches
    a prefix of the path.

    This will not work if the end of the path is a NodeFile. Use
    FT_traversePathFromFile & FT_traversePathFile instead.
*/
static NodeDir FT_traversePathFromDir(char* path, NodeDir curr) {
   NodeDir found;
   size_t i;

   assert(path != NULL);

   if (curr == NULL)
      return NULL;

   else if (!strcmp(path, NodeDir_getPath(curr)))
      return curr;


   else if (!strncmp(path, NodeDir_getPath(curr), 
   strlen(NodeDir_getPath(curr)))) {
      for (i = 0; i < NodeDir_getNumChildDirs(curr); i++) {
         found = FT_traversePathFromDir(path,
                                NodeDir_getChildDir(curr, i));
         if(found != NULL)
            return found;
      }
      return curr;
   }
   return NULL;
}


/*
   Returns the farthest NodeDir reachable from root following a given
   path, or NULL if there is no Node in the hierarchy that matches a
   prefix of the path.
*/
static NodeDir FT_traversePathDir(char* path) {
    assert(path != NULL);

    if (rootFile != NULL) return NULL;

    return FT_traversePathFromDir(path, rootDir);
}


/*
    Recursive helper function for FT_traversePathFile

    Starting at the parameter curr, traverses as far down
    the hierarchy as possible, just considering NodeDir's, while
    still matching the path parameter.

    Returns a pointer to the farthest matching NodeDir down that path,
    or NULL if there is no NodeDir in curr's hierarchy that matches
    a prefix of the path.
*/
static NodeDir FT_traversePathFromFile(char* path, NodeDir curr) {
    NodeDir found;
    size_t i;

    assert(path != NULL);

    if (curr == NULL)
        return NULL;

    if (NodeDir_hasChildFile(curr, path, NULL) == 1) return curr;
    if (NodeDir_hasChildDir(curr, path, NULL) == 1) return curr;

    if (!strncmp(path, NodeDir_getPath(curr), 
    strlen(NodeDir_getPath(curr)))) {
        for (i = 0; i < NodeDir_getNumChildDirs(curr); i++) {
            found = FT_traversePathFromFile(path, 
            NodeDir_getChildDir(curr, i));
            if(found != NULL)
                return found;
        }
        return curr;
    }
    return NULL;
}


/*
    Returns the farthest NodeDir reachable from root following a given
    path, or NULL if there is no Node in the hierarchy that matches a
    prefix of the path.

    To clarify: even if NodeFile is in the hierarchy, this will return
    its parent NodeFile.
*/
static NodeDir FT_traversePathFile(char* path) {
    assert(path != NULL);

    if (rootFile != NULL) return NULL;

    return FT_traversePathFromFile(path, rootDir);
}


/**********************************************************************/
/* Inserting path */
/**********************************************************************/


/*
    Helper function for FT_insertDir.

    Inserts a new path of NodeDirs into the tree rooted at parent, or,
    if parent is NULL, as the root of the data structure.

    If a node representing path already exists, returns ALREADY_IN_TREE

    If there is an allocation error in creating any of the new nodes or
    their fields, returns MEMORY_ERROR

    If there is an error linking any of the new nodes,
    returns PARENT_CHILD_ERROR

    Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPathDir(char* path, NodeDir parent) {
    NodeDir curr = parent;
    NodeDir firstNew = NULL;
    NodeDir new;
    char* copyPath;
    char* restPath = path;
    char* dirToken;
    char* fileCheck;
    char* fileCheckChange;
    int result;
    size_t newCount = 0;

    assert(path != NULL);

    if (curr == NULL) {
        /* if root exists but we have a NULL */
        if (rootDir != NULL || rootFile != NULL) {
            return CONFLICTING_PATH;
        }
    }
    else { /* if inserting path from already existing curr */

        if(!strcmp(path, NodeDir_getPath(curr)))
            return ALREADY_IN_TREE;

        /* checking if this final dir has a child nodeFile that overlaps
        w/ a prefix of the path of nodeDir we're trying to insert. */
        fileCheck = malloc(strlen(path)+1);
        if (fileCheck == NULL) return MEMORY_ERROR;
        fileCheck = strcpy(fileCheck, path);
        if (fileCheck == NULL) return MEMORY_ERROR;
        

        if (NodeDir_hasChildFile(curr, path, NULL) == 1) {
            free(fileCheck);
            return ALREADY_IN_TREE;
        }

        /* use fileCheckChange to turn fileCheck into path of what
        this childFile would be */
        fileCheckChange = fileCheck;
        fileCheckChange += strlen(NodeDir_getPath(curr)) + 1;
        fileCheckChange = strstr(fileCheckChange, "/");

        if (fileCheckChange != NULL) *fileCheckChange = '\0';

        if (NodeDir_hasChildFile(curr, fileCheck, NULL) == 1) {
            free(fileCheck);
            return PARENT_CHILD_ERROR;
        }
        free(fileCheck);
        
         
        
        restPath += (strlen(NodeDir_getPath(curr)) + 1);
    }
    /* check for empty string at end of path */
    if (*(restPath+strlen(restPath)-1) == '/') {
        return PARENT_CHILD_ERROR;
    }

    copyPath = malloc(strlen(restPath)+1);
    if(copyPath == NULL)
        return MEMORY_ERROR;
    strcpy(copyPath, restPath);
    dirToken = strtok(copyPath, "/");

    while(dirToken != NULL) {
        /* check for empty string in path */
        if (*(dirToken-1) == '/') {
            free(copyPath);
            return PARENT_CHILD_ERROR;
        }
        new = NodeDir_create(dirToken, curr);
        newCount++;

        if(firstNew == NULL)
            firstNew = new;
        else {
            result = FT_linkParentToChildDir(curr, new); 
            /* hopefully success */
            if(result != SUCCESS) {
                (void) NodeDir_destroy(new);
                (void) NodeDir_destroy(firstNew);
                free(copyPath);
                return result;
            }
        }

        if(new == NULL) {
            (void) NodeDir_destroy(firstNew);
            free(copyPath);
            return MEMORY_ERROR;
        }

        curr = new;
        dirToken = strtok(NULL, "/");
    }


   free(copyPath);

   if(parent == NULL) {
      rootDir = firstNew;
      countDirs = newCount;
      return SUCCESS;
   }
   else {
      result = FT_linkParentToChildDir(parent, firstNew);
      if(result == SUCCESS)
         countDirs += newCount;
      else
         (void) NodeDir_destroy(firstNew);

      return result;
   }
}


/* see ft.h for specification */
int FT_insertDir(char *path) {
    NodeDir curr;
    int result;

    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;
    curr = FT_traversePathDir(path);
    result = FT_insertRestOfPathDir(path, curr);
    return result;
}


/*
    Helper function for FT_insertPath.

    Inserts a new path of NodeDirs with a final NodeFile into the 
    tree rooted at parent, or, if parent is NULL, a NodeFile as the root

    Adds contents and length to final NodeFile.

    If a node representing path already exists, returns ALREADY_IN_TREE

    If there is an allocation error in creating any of the new nodes or
    their fields, returns MEMORY_ERROR

    If there is an error linking any of the new nodes,
    returns PARENT_CHILD_ERROR

    Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPathFile(char* path, NodeDir parent, 
void* contents, size_t length) {
    NodeDir curr = parent;
    NodeDir firstNew = NULL;
    NodeDir new;
    NodeFile finalFile;
    char* copyPath;
    char* restPath = path;
    char* dirToken;
    char* findFile;
    char* fileCheck;
    char* fileCheckChange;
    int result;
    size_t newCount = 0;

    assert(path != NULL);

    if (curr == NULL) {
        if (rootDir != NULL || rootFile != NULL)
            return CONFLICTING_PATH;
    }
    else {
        if (NodeDir_hasChildFile(curr, path, NULL) == 1 || 
        NodeDir_hasChildDir(curr, path, NULL) == 1)
            return ALREADY_IN_TREE;

        /* checking if this final dir has a child nodeFile that overlaps
        w/ a prefix of the path of nodeDir we're trying to insert. */
        fileCheck = malloc(strlen(path)+1);
        if (fileCheck == NULL) return MEMORY_ERROR;
        fileCheck = strcpy(fileCheck, path);
        if (fileCheck == NULL) return MEMORY_ERROR;

        fileCheckChange = fileCheck;
        fileCheckChange += strlen(NodeDir_getPath(curr)) + 1;
        fileCheckChange = strstr(fileCheckChange, "/");

        if (fileCheckChange != NULL) {
            *fileCheckChange = '\0';

            if (NodeDir_hasChildFile(curr, fileCheck, NULL) == 1) {
                free(fileCheck);
                return NOT_A_DIRECTORY;
            }
                
        }
        free(fileCheck);

        restPath += (strlen(NodeDir_getPath(curr)) + 1);
    }
    /* check for empty string at end of path */
    if (*(restPath+strlen(restPath)-1) == '/') {
        return PARENT_CHILD_ERROR;
    }

    copyPath = malloc(strlen(restPath)+1);
    if(copyPath == NULL)
        return MEMORY_ERROR;
    strcpy(copyPath, restPath);
    dirToken = strtok(copyPath, "/");

    /* special case - checking if this is where we should put in file */
    findFile = strstr(restPath, "/");
    if (findFile == NULL) {
        /* if file should be root */
        if (parent == NULL) {
            finalFile = NodeFile_create(path, NULL, contents, length);

            if (finalFile == NULL) {
                (void) NodeFile_destroy(finalFile);
                free(copyPath);
                return MEMORY_ERROR;
            }

            rootFile = finalFile;
            free(copyPath);
            return SUCCESS;
        } 
        /* adding file to end of hiararchy */
        finalFile = NodeFile_create(restPath, parent, contents, length);
        if (finalFile == NULL) {
            (void) NodeFile_destroy(finalFile);
            free(copyPath);
            return MEMORY_ERROR;
        }
        result = FT_linkParentToChildFile(parent, finalFile);
        if (result != SUCCESS) (void) NodeFile_destroy(finalFile);

        free(copyPath);
        return result;
    }

    while(dirToken != NULL) {
        /* check for empty string in path */
        if (*(dirToken-1) == '/') {
            free(copyPath);
            return PARENT_CHILD_ERROR;
        }
        /* checking if we have the final file */
        if (findFile == NULL) {
            finalFile = NodeFile_create(dirToken, curr, contents, 
            length);
            if (finalFile == NULL) {
                (void) NodeDir_destroy(firstNew);
                free(copyPath);
                return MEMORY_ERROR;
            }

            result = FT_linkParentToChildFile(curr, finalFile);
            if(result != SUCCESS) {
                (void) NodeFile_destroy(finalFile);
                (void) NodeDir_destroy(firstNew);
                free(copyPath);
                return result;
            }
            break;
        }

        /* if node isn't final file but just lame NodeDir */
        new = NodeDir_create(dirToken, curr);
        newCount++;

        if (firstNew == NULL) 
        firstNew = new;
        else {
            result = FT_linkParentToChildDir(curr, new);
            if(result != SUCCESS) {
                (void) NodeDir_destroy(new);
                (void) NodeDir_destroy(firstNew);
                free(copyPath);
                return result;
            }
        }

        if(new == NULL) {
            (void) NodeDir_destroy(firstNew);
            free(copyPath);
            return MEMORY_ERROR;
        }

        curr = new;
        dirToken = strtok(NULL, "/");

        findFile++;
        findFile = strstr(findFile, "/");
    }

   free(copyPath);

   if(parent == NULL) {
      rootDir = firstNew;
      countDirs = newCount;
      return SUCCESS;
   }
   else {
      result = FT_linkParentToChildDir(parent, firstNew);
      if(result == SUCCESS)
         countDirs += newCount;
      else
         (void) NodeDir_destroy(firstNew);

      return result;
   }
}


/*  See ft.h for specification. */
int FT_insertFile(char *path, void *contents, size_t length) {
    NodeDir curr;
    int result;

    assert(path != NULL);

    if(!isInitialized)
        return INITIALIZATION_ERROR;
    curr = FT_traversePathFile(path);
    result = FT_insertRestOfPathFile(path, curr, contents, length);
    return result;
}


/**********************************************************************/
/* Simple API functions */
/**********************************************************************/


/*  See ft.h for specification. */
boolean FT_containsDir(char *path) {
    NodeDir curr;

    assert(path != NULL);

    if(!isInitialized)
        return FALSE;

    curr = FT_traversePathDir(path);

    if (curr == NULL)
        return FALSE;
    else if(strcmp(path, NodeDir_getPath(curr)))
        return FALSE;
    else
        return TRUE;
}


/*  See ft.h for specification. */
boolean FT_containsFile(char *path) {
    NodeDir curr;

    assert(path != NULL);

    if(!isInitialized)
        return FALSE;

    /* edge case - if root is a file */
    if (rootFile != NULL) {
        if (!strcmp(NodeFile_getPath(rootFile), path))
            return TRUE;
        else 
            return FALSE;
    }

    curr = FT_traversePathFile(path);

    if (curr == NULL)
        return FALSE;
    else if (NodeDir_hasChildFile(curr, path, NULL) == 1)
        return TRUE;
    else
        return FALSE;
}


/* see ft.h for specification */
int FT_rmDir(char *path) {
    NodeDir curr;

    assert(path != NULL);
    if (!isInitialized) return INITIALIZATION_ERROR;

    curr = FT_traversePathDir(path);

    if (curr == NULL) 
        return NO_SUCH_PATH;
    else if (NodeDir_hasChildFile(curr, path, NULL) == 1)
        return NOT_A_DIRECTORY;
    else if (strcmp(NodeDir_getPath(curr), path))
        return NO_SUCH_PATH;
    else {
        if (NodeDir_getParent(curr) == NULL) {
            FT_removePathFromDir(curr);
            rootDir = NULL;
            return SUCCESS;
        }
        NodeDir_unlinkChildDir(NodeDir_getParent(curr), curr);
        FT_removePathFromDir(curr);
        return SUCCESS;        
    } 
    
}


/* see ft.h for specification */
int FT_rmFile(char *path) {
    NodeDir curr;
    NodeFile child;
    size_t childIndex;

    assert(path != NULL);
    if (!isInitialized) return INITIALIZATION_ERROR;

    /* edge case - root is file */
    if (rootFile != NULL) {
        if (!strcmp(NodeFile_getPath(rootFile), path)) {
            (void) NodeFile_destroy(rootFile);
            rootFile = NULL;
            return SUCCESS;
        }
        return NO_SUCH_PATH;
    }

    curr = FT_traversePathFile(path);

    if (curr == NULL)
        return NO_SUCH_PATH;
        
    else if (NodeDir_hasChildFile(curr, path, &childIndex) == 1) {
        child = NodeDir_getChildFile(curr, childIndex);
        NodeDir_unlinkChildFile(curr, child);
        (void) NodeFile_destroy(child);
        return SUCCESS;
    }
    else if (NodeDir_hasChildDir(curr, path, NULL))
        return NOT_A_FILE;
    else
        return NO_SUCH_PATH;    
}


/* see ft.h for specification */
void *FT_getFileContents(char *path) {
    NodeDir curr;
    size_t childIndex;

    assert(path != NULL);

    /* edge case - root is file */
    if (rootFile != NULL) {
        if (!strcmp(NodeFile_getPath(rootFile), path)) {
            return NodeFile_getContents(rootFile);
        }
        return NULL;
    }


    curr = FT_traversePathFile(path);

    if (NodeDir_hasChildFile(curr, path, &childIndex) == 1)
        return NodeFile_getContents(NodeDir_getChildFile(curr, 
        childIndex));
    
    return NULL;
}


/* see ft.h for specification */
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength) {
    NodeDir curr;
    size_t childIndex;

    assert(path != NULL);

    /* edge case - root is file */
    if (rootFile != NULL) {
        if (!strcmp(NodeFile_getPath(rootFile), path)) {
            return NodeFile_replaceContents(rootFile, 
            newContents, newLength);
        }
        return NULL;
    }

    curr = FT_traversePathFile(path);

    if (NodeDir_hasChildFile(curr, path, &childIndex) == 1)
        return NodeFile_replaceContents(NodeDir_getChildFile(curr, 
        childIndex), newContents, newLength);
    
    return NULL;
}


/* see ft.h for specification */
int FT_init(void) {
    if (isInitialized) return INITIALIZATION_ERROR;

    isInitialized = 1;
    rootDir = NULL;
    rootFile = NULL;
    countDirs = 0;
    return SUCCESS;
}


/* see ft.h for specification */
int FT_destroy(void) {
    if (!isInitialized) return INITIALIZATION_ERROR;

    if (rootDir == NULL && rootFile == NULL) {
        isInitialized = 0;
        return SUCCESS;
    }

    if (rootFile != NULL) {
        (void) NodeFile_destroy(rootFile);
        rootFile = NULL;
        isInitialized = 0;
        return SUCCESS;
    }

    FT_removePathFromDir(rootDir);
    rootDir = NULL;
    isInitialized = 0;
    return SUCCESS;
}


/* see ft.h for specification */
int FT_stat(char *path, boolean* type, size_t* length) {
    NodeDir curr;
    size_t childIndex;

    assert(path != NULL);
    assert(type != NULL);
    assert(length != NULL);

    if (!isInitialized) return INITIALIZATION_ERROR;

    /* edge case - root is file */
    if (rootFile != NULL) {
        if (!strcmp(NodeFile_getPath(rootFile), path)) {
            *type = TRUE;
            *length = NodeFile_getLength(rootFile);
            return SUCCESS;
        }
        return NO_SUCH_PATH;
    }

    curr = FT_traversePathFile(path);

    if (curr == NULL) return NO_SUCH_PATH;

    if (NodeDir_hasChildDir(curr, path, NULL) == 1) {
        *type = FALSE;
        return SUCCESS;
    } else if (NodeDir_hasChildFile(curr, path, &childIndex) == 1) {
        *type = TRUE;
        *length = NodeFile_getLength(NodeDir_getChildFile(curr, 
        childIndex));
        return SUCCESS;
    }
    return NO_SUCH_PATH;
}


/**********************************************************************/
/* toString */
/**********************************************************************/


/*
   Performs a pre-order traversal of the NodeDirs' in the tree 
   rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(NodeDir n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) {
      (void) DynArray_set(d, i, n);
      i++;
      for(c = 0; c < NodeDir_getNumChildDirs(n); c++)
         i = FT_preOrderTraversal(NodeDir_getChildDir(n, c), d, i);
   }
   return i;
}


/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.

   Accumulates all dirs and files.
*/
static void FT_strlenAccumulate(NodeDir n, size_t* pAcc) {
    size_t i;

    assert(pAcc != NULL);
    assert(n != NULL);

    *pAcc += strlen(NodeDir_getPath(n)) + 1;

    for (i = 0; i < NodeDir_getNumChildFiles(n); i++) {
        *pAcc += strlen(NodeFile_getPath(NodeDir_getChildFile(n, i)))
         + 1;
    }
}


/*
   Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string.
*/
static void FT_strcatAccumulate(NodeDir n, char* acc) {
    size_t i;
    

    assert(acc != NULL);
    assert(n != NULL);

    strcat(acc, NodeDir_getPath(n));
    strcat(acc, "\n");

    for (i = 0; i < NodeDir_getNumChildFiles(n); i++) {
        strcat(acc, NodeFile_getPath(NodeDir_getChildFile(n, i)));
        strcat(acc, "\n");
    }
}


/*
  Returns a string representation of the
  data structure, or NULL if the structure is
  not initialized or there is an allocation error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char *FT_toString() {
    DynArray_T nodes;
    size_t totalStrlen = 1;
    char* result = NULL;

    if (!isInitialized)
        return NULL;

    /* edge case - root is file */
    if (rootFile != NULL)
        return strcat((char *) NodeFile_getPath(rootFile), 
        (const char*)  "\n");
    
    nodes = DynArray_new(countDirs);

    (void) FT_preOrderTraversal(rootDir, nodes, 0);

    DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate, 
    (void*) &totalStrlen);

    result = malloc(totalStrlen);

    if(result == NULL) {
        DynArray_free(nodes);
        return NULL;
    }
    *result = '\0';

    if (rootDir == NULL && rootFile == NULL) {
        DynArray_free(nodes);
        return result;
    }
    DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate, 
    (void *) result);

    DynArray_free(nodes);
    return result;
}