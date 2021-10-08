/*--------------------------------------------------------------------*/
/* checker.c                                                          */
/* Authors: Jake Intrater and Anton Stengel (and cmoretti of course)  */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checker.h"


/* see checker.h for specification */
boolean Checker_Node_isValid(Node n) {
   Node parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
   size_t index;

   /* Sample check: a NULL pointer is not a valid Node */
   if(n == NULL) {
      fprintf(stderr, "Node is a NULL pointer\n");
      return FALSE;
   }
   /* Check that node has non-null path */
   npath = Node_getPath(n);
   if (npath == NULL) {
      fprintf(stderr, "Node has a NULL path\n");
      return FALSE;
   }

   index = Node_getNumChildren(n);

   if (index > 1) {
      for (i = 0; i < (index - 1); i++) {
         if (Node_getChild(n,i) == NULL 
         || Node_getChild(n,i+1) == NULL) {
            fprintf(stderr, "N has null children\n");
            return FALSE;
         }
         if(strcmp(
            Node_getPath(Node_getChild(n, i)),
            Node_getPath(Node_getChild(n, i+1))) >= 0) {
            fprintf(stderr, 
            "Children are not in lexicographic order\n");
            return FALSE;
         }
      }
   }  
   parent = Node_getParent(n);
   if(parent != NULL) {
      
      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }
   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Tracks the number of nodes visited with counter pointer.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.
*/
static boolean Checker_treeCheck(Node n, size_t* counter) {
   size_t c;
   assert(counter != NULL);

   if(n != NULL) {
      (*counter)++;

      /* Sample check on each non-root Node: Node must be valid */
      /* If not, pass that failure back up immediately */
      if(!Checker_Node_isValid(n))
         return FALSE;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node child = Node_getChild(n, c);
         if(Node_compare(Node_getParent(child),n) != 0) {
            fprintf(stderr, 
            "Node is not linked to its proper parent\n");
            return FALSE;
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!Checker_treeCheck(child, counter))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checker.h for specification */
boolean Checker_DT_isValid(boolean isInit, Node root, size_t count) {
   size_t counter;
   boolean toReturn;
   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit) {
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
      if(root != NULL) {
         fprintf(stderr, "Not initialized, but root is not null\n");
         return FALSE;
      }
   }
   /* Check to see that root and count match */
   if(isInit) {
      if (root == NULL && count != 0) {
         fprintf(stderr, "Empty tree with non-zero count\n");
         return FALSE;
      }
   }
   if (root != NULL) {
      if (Node_getParent(root) != NULL) {
         fprintf(stderr, 
         "Root of the DT is pointing to a node with a parent\n");
         return FALSE;
      }
   }
   

   /* Now checks invariants recursively at each Node from the root. */
   counter = 0;
   toReturn = Checker_treeCheck(root, &counter);
   if (count != counter && toReturn) {
      fprintf(stderr, "Count variable given is incorrect\n");
      fprintf(stderr, "Count given: %lu\n Our Counter %lu\n",
       count, counter);
      return FALSE;
   }
   return toReturn;
}
