/*****************************************************************

            Filename    :  basic_tree.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Class containing some basics of a tree.

******************************************************************/


#ifndef BASIC_TREE_H
#define BASIC_TREE_H

#include <iostream>
#include "node.h"
#include "edge.h"
#include "def.glb"

class basic_tree  {

public:

   basic_tree();
   ~basic_tree();

   int     getnode_edge_first(int i) const;
   int     getnode_edge_sec(int i) const;

   int     get_node_nb() const;
   double  get_Xcoord(int i) const;
   double  get_Ycoord(int i) const;   
   char*   get_node_info(int i) const;
   char*   get_node_short(int i) const;
   int     get_node_colour(int node) const;
   int     get_under_colour(int node) const;

   int     is_standard() const;
   int     is_displayed(int node) const;
   int     is_highlighted(int node) const;

   void    set_standard(int t);
   void    set_node_colour(int node,int colour);
   void    set_under_colour(int node,int colour);

   void    display(int node,int number);
   void    highlight(int node,int number);

   int     tree_empty();
   
protected:

   int      node_nb;
   int      edge_nb;
   int      standard;
   
   node     *root;

   
   edge*    edge_array;
   node**   node_array;
   int      array_size;   

};


#endif
