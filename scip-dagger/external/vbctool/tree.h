/************************************************************************

            Filename    :  tree.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  A derived class from basic_tree. Computes
	                   and manages the coordinates of all nodes
			   of a tree for a nice layout. Uses an
			   algorithm by John Q. Walker published in:
			   Software-Practice and Experience, vol. 20(7),
			   685-705 (July 1990) under the titel:
			   A Node-positioning Algorithm for General Trees.

************************************************************************/


#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <fstream>
#include "basic_tree.h"
#include "queue.h"

class tree : public basic_tree  {
   friend char     *operator>>(char*, tree&);
   friend node*    allocate_tree(char* ,tree &,int &);
   friend int      enter_edge(std::ifstream &,tree &,int);
   friend int      set_node_info(int,char*,tree &);
   friend int      add_node_info(int,char*,tree &);
public:

   tree();
   ~tree();


   void    positiontree(double*, double*);
   void    newPositionLevel(int, double*, double*);

   void    enter_new_node(int, int, int);
   void    clean_tree();
   
   int     get_level_count() const;
   int     get_actual_level_separation() const;
   double  get_max_X_coord() const;
   double  get_max_Y_coord() const;

   
   static int get_sib_separation();
   static int get_level_separation();
   static int get_subtree_separation();

   void set_separation_values(int sibling,int level,int subtree);

private:

   double   root_X_coord;
   double   root_Y_coord;

   int      level_count;
   int      actual_level_sep;
   double   max_X_coord;
   double   max_Y_coord;

   
   static int sibling_separation;
   static int level_separation;
   static int subtree_separation;
      
   void   compute_coord(node *,double*,double*,double*);
   void   firstwalk(node &,int,node*[]);
   void   secondwalk(node *, int, double,double*,double*,double*);

   void   apportion(node &,int);
   node*  get_left_most(node *,int,int);
   
   void   get_max_XYvalue(node*, double*, double*, double*);

   void   deapth_first_search(node *,int);
   void   del_deapth_first(node *);
   int    mean_node_size(node *,node *);
   void   introduce_child(node *,node *);

};


#endif
