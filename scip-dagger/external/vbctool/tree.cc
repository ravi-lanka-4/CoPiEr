/*****************************************************************

            Filename    :  tree.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.

******************************************************************/


#include "tree.h"
#include <iostream>
#include <string>
#include <cmath>

/************************************************************************
                         STATIC

             This part contains all static files
*************************************************************************/





int tree::sibling_separation = 4;
int tree::level_separation = 2;
int tree::subtree_separation = 4;


/************************************************************************
                     get_sib_separation
*************************************************************************/


int tree::get_sib_separation()
{
   return sibling_separation;
}


/************************************************************************
                    get_level_separation
*************************************************************************/


int tree::get_level_separation()
{
   return level_separation;
}


/************************************************************************
                    get_subtree_separation
*************************************************************************/


int tree::get_subtree_separation()
{
   return subtree_separation;
}



/************************************************************************
                    set_separation_values
*************************************************************************/


void tree::set_separation_values(int sibling,int level,int subtree)
{
   sibling_separation = sibling;
   level_separation = level+2;            // +2 for keeping distance 
   actual_level_sep = level+2;
   subtree_separation = subtree;
}








/************************************************************************
                         PUBLIC

             This part contains all public files
*************************************************************************/



/************************************************************************
                         constructor
*************************************************************************/


tree::tree()
   : basic_tree()
{

   root_X_coord = 0.0;
   root_Y_coord = 0.0;

   level_count = 0;
   actual_level_sep = level_separation;
   max_X_coord = 0.0;
   max_Y_coord = 0.0;

}



/************************************************************************
                         destructor
*************************************************************************/


tree::~tree()
{
//   if (root != NULL)
//      del_deapth_first(root);
}






/************************************************************************
                         positiontree
*************************************************************************/


void tree::positiontree(double *x_max, double *y_max)
 
   // Calls the procedure compute_coord for computing the coordinates
   // of the tree nodes. Returns the values of the maximum X and Y
   // coordinates in order to adjust a coordinate system of proper size
   // to a drawing of the tree.
{
   
   (*x_max)     = 0.0;        // maximum x coordinate
   (*y_max)     = 0.0;        // maximum y coordinate
   double x_min = 0.0;        // minimum x coordinate
   double help  = 0.0;

   compute_coord(root,x_max,y_max,&x_min);

                              // The smallest x coordinate might be negative.
			      // If this is the case, the tree is moved into
			      // positive direction of the x-axis until no
			      // node has a negative x-coordinate. This
			      // is necessary, since the coordinate system,
			      // in which the tree is drawn, is only positive.
			      // Observe that the value of the maximum 
			      // x-coordinate has to be adjusted as well.
   if (x_min < 0)
   {
      for (int i = 1; i <= node_nb; i++)
      {
         help = node_array[i]->get_Xcoord();
         node_array[i]->set_Xcoord(help - x_min);
      }
      (*x_max) -= x_min;
   }
   max_X_coord = (*x_max);
   max_Y_coord = (*y_max);
}





/************************************************************************
                     newPositionLevel
*************************************************************************/


void tree::newPositionLevel(int new_scaler,double* x_max, double *y_max)

   // This function is specially introduced to fit the needs of the
   // TreeInterace. In case that a user of the TreeInterface cooses
   // a new level separation value via a popup menue called "Scaler",
   // it is not necessary to compute the coordinates completely new,
   // since only the y-coordinates have to be adapted. This is done
   // here in the function newPositionLevel.
   // Furthermore, since changing the level speration value changes the
   // size of the drawing, this function returns the maximal coordinate
   // values so the coordinate system can be addapted to the new
   // drawing of the tree.
{
   double help = 0.0;

   (*y_max) = 0.0;
   (*x_max) = max_X_coord;

                                 // For all nodes, compute the 
				 // y-coordinate again.
   for (int i = 1; i <= node_nb; i++)
   {
      help = node_array[i]->get_Ycoord();
      help = (help / actual_level_sep) * new_scaler;
      node_array[i]->set_Ycoord(help);
      if (help > (*y_max))
         (*y_max) = help;
   }
                                 // Memorize the new values.
   actual_level_sep = new_scaler;
   level_separation = new_scaler;
   max_Y_coord = (*y_max);
}
      




/************************************************************************
                      enter_new_node
*************************************************************************/

void tree::enter_new_node(int father,int new_node,int colour)

   // This function is specially introduced to fit the needs of the
   // TreeInterace. The TreeInterface offers the option to add nodes
   // to already existing trees. So this public function provides
   // the TreeInterface a tool, for adding nodes.
    
{
   int  newroot = FALSE;

                                 // The tree is empty. Construct space
				 // to hold up to a hundred nodes. This
				 // is done since the computation of the
				 // coordinates is made fast with the help
				 // of such an array. Furthermore, we do
				 // not want every time that a node is 
				 // added to the tree, rebuild that array.
   if (root == NULL)
   {
      array_size = 100;
      node_array = new node*[array_size+1];
      edge_array = new edge[array_size];
    
      for (int i = 1;  i <= array_size; i++)
         node_array[i] = new node(i);
      root = node_array[1];
      newroot = TRUE;
   }

                                 // The tree is not empty, but the array
				 // is full. Add space for 100 more
				 // nodes to the array.
   else if (root != NULL && (node_nb == array_size || new_node > array_size))
   {
       int _addSpace = 100;
       if ( new_node - array_size + 1 > 100)
       {
           int _faktor = (int) ceil( (new_node - array_size + 1)/100 );
           _addSpace *= _faktor;
       }
               
           
      node** new_node_array = new node*[array_size+_addSpace+1];
      edge* new_edge_array = new edge[array_size+_addSpace];

      for (int i = 1;  i <= array_size; i++)
      {
         new_node_array[i] = node_array[i];
      }

      for (int k = 1;  k <= (array_size-1); k++)
      {
         new_edge_array[k] = edge_array[k];
      }
                                 // Changed after using gcc 2.7.0      
      for (int j = array_size+1; j <= (array_size+_addSpace); j++)
         new_node_array[j] = new node(j);
      delete[] node_array;
      delete[] edge_array;
      node_array = new_node_array;
      edge_array = new_edge_array;

      array_size = array_size + _addSpace;
   }

   if (father != 0)
                                 // The node is not the root.
   {
      if (father < new_node)
                                 // Include node into the structure
				 // of the tree.
      {
         node_array[new_node]->set_parent(node_array[father]);
         introduce_child(node_array[father],node_array[new_node]);
         edge_array[node_nb].set_first(father);
         edge_array[node_nb].set_second(new_node);
         node_nb++;
	 edge_nb++;
      }
   }
   else if (newroot)
      node_nb++;

                                 // Set the nodes colour.
   node_array[new_node]->set_colour(colour);
   node_array[new_node]->set_underlying_colour(colour);
}



/************************************************************************
                      clean_tree
*************************************************************************/



void tree::clean_tree()

   // Empties all information stored by the class tree, if a new tree
   // has to be saved.
{
   if (root != NULL)
   {
      root = NULL;

      root_X_coord = 0.0;
      root_Y_coord = 0.0;

      level_count = 0;
      actual_level_sep = level_separation;
      max_X_coord = 0.0;
      max_Y_coord = 0.0;

      delete[] edge_array;


      for (int i = 1; i <= (array_size == 0 ? node_nb : array_size); i++)
         delete node_array[i];
      delete[] node_array;

      edge_array = NULL;
      node_array = NULL;

      node_nb = 0;
      edge_nb = 0;

      array_size = 0;
   }
}



/************************************************************************
                      get_level_count
*************************************************************************/


int  tree::get_level_count() const

   // Returns the number of levels in the tree.
{
   return level_count;
}




/************************************************************************
                  get_actual_level_separation
*************************************************************************/


int  tree::get_actual_level_separation() const

   // Returns the actual level_separation.
{
   return actual_level_sep;
}





/************************************************************************
                        get_max_X_coord
*************************************************************************/


double  tree::get_max_X_coord() const

    // Returns the maximal x-coordinate of the tree.
{
   if (root != NULL)
      return max_X_coord;
   else
   {
      std::cout << "tree::get_max_X_coord: empty tree" << std::endl;
      return 0.0;
   }
}



/************************************************************************
                        get_max_Y_coord
*************************************************************************/


double  tree::get_max_Y_coord() const

    // Returns the maximal x-coordinate of the tree.
{
   if (root != NULL)
      return max_Y_coord;
   else
   {
     std::cout << "tree::get_max_Y_coord: empty tree" << std::endl;
      return 0.0;
   }
}







/************************************************************************
                         PRIVAT

             This part contains all privat files
*************************************************************************/



      



/************************************************************************
                      compute_coord
*************************************************************************/


void tree::compute_coord(node *check_node,double* x_max,
                         double*y_max,double*x_min)

    // Computes the X and Y coordinates of all nodes in the tree. 
{
   if (check_node != NULL)
   {
      level_count = 0;
                                 // Scan the complete tree in deapth-first
				 // manner in order to determine the number
				 // of levels of the tree and setting
				 // previous y-coordinates.
      deapth_first_search(check_node,0);

      node **level_array;        // Array of pointers to nodes.
                                 // An entry at position i in level_array
				 // contains a pointer to the last visited
				 // node at this level (the so-called
				 // previous node). If another node 
				 // is checked at level i, than level_array[i]
				 // points to the left neighbor of the node.
      level_array = new node*[level_count+1];
      

      for (int i = 0; i <= level_count; level_array[i++] = NULL);

                                 // Do a post-order walk in order to assign to
				 // every node a preliminary x-coordinate,
				 // held in the field prelim_x of every node.
				 // In adition, internal nodes are given
				 // modifiers, which will be used to move
				 // their offsprings to the right.
      firstwalk(*check_node,0,level_array);

                                
      root_X_coord = 0;          // Set the x-coordinate of the root by default 0.
      root_Y_coord = 0;          // Set the y-coordinate of the root by default 0. 

                                 // Do a pre-order walk in order to give each
				 // node its final x-coordinate by summing
				 // its preliminary x-coordinates and the
				 // modifiers of all nodes ancestors.
      secondwalk(check_node,0,0,x_max,y_max,x_min);

      delete[] level_array;
   }
}





/************************************************************************
                      firstwalk
*************************************************************************/


void tree::firstwalk(node &check_node,int level,node* level_array[])

   // Does a post-order walk in order to assign to every node a
   // preliminary x-coordinate, held in the field prelim_x of every node.
   // In adition, internal nodes are given modifiers, which will be
   // used to move their offsprings to the right.
{
   
                                 // Set the pointer to the previous node
				 // at this level.
   check_node.set_leftneighbor(level_array[level]);
   level_array[level] = &check_node;

   check_node.set_modifier(0);

   if (check_node.is_leaf())
   {
      if (check_node.has_left_sib())
                                 // Determine the preliminary x-ccordinate
				 // based on the following facts:
				 // the prelim_x of the left sibling,
				 // the separation between the sibling nodes,
				 // and the meansize of the left sibling
				 // and the current node.
      {
	 check_node.set_prelim(check_node.get_leftsibling()->get_prelim()
	                       + tree::get_sib_separation()
			       + mean_node_size(check_node.get_leftsibling(),
			                        &check_node));
      }
      else
                                 // There is no sibling on the left, that
				 // we have to take care about.
         check_node.set_prelim(0);
   }

   else
                                 // This node is not a leaf, so call this
				 // procedure recursively for each for
				 // its children. 
   {
      node    *left_most = NULL;
      node    *right_most = NULL;
      double   midpoint = 0;
      

      left_most = right_most = check_node.get_firstchild();
      firstwalk(*left_most,level+1,level_array);

      while (right_most->has_right_sib())
      {  
         right_most = right_most->get_rightsibling();
	 firstwalk(*right_most,level+1,level_array);
      }

      midpoint = (left_most->get_prelim() + right_most->get_prelim())/2;

      if (check_node.has_left_sib())
      {
                                 // Determine the preliminary x-ccordinate
				 // based on the following facts:
				 // the prelim_x of the left sibling,
				 // the separation between the sibling nodes,
				 // and the meansize of the left sibling
				 // and the current node.
         check_node.set_prelim(check_node.get_leftsibling()->get_prelim()
	                       + tree::get_sib_separation()
			       + mean_node_size(check_node.get_leftsibling(),
						&check_node));

                                 // Since check_node has to be moved by the
				 // value prelim_x to the right, all its
				 // descendants have to do the same.
				 // So set the modifier of the checked node by
				 // prelim_x. In order to center check_node
				 // over its children, subtract midpoint.
	 check_node.set_modifier(check_node.get_prelim() - midpoint);

	                         // The subtree of check_node may still 
				 // overlay the subtree of its left sibling.
				 // In order to find this out, the leftmost
				 // descendants of check_node on each level
				 // are examined and if necessary the complete
				 // tree is moved to the right.
	 apportion(check_node,level);
      }

      else
                                 // Center check_node over its children.
         check_node.set_prelim(midpoint);
   }

}






/************************************************************************
                      secondwalk
*************************************************************************/


void   tree::secondwalk(node *check_node, int level, double modsum,
                        double* x_max,double*y_max,double*x_min)

   // Rekursive procedure which does a pre-order walk in order to give
   // each node its final x-coordinate by summing its preliminary
   // x-coordinates and the modifiers of all nodes ancestors.
   // If the actual position of an interior node is right of its
   // preliminary place (stored in prelim_x), the subtree rooted at the
   // node must be moved to the right, so that the children of the node
   // are centered around the father. Rather than immediately readjust
   // all nodes of the subtree, each node remembers the distance to the
   // preliminary place in its modifier field mod. In this second pass
   // down the tree, the modifiers are accumulated and applied to every node.
{

                                 // Compute the x- and y coordinates of
				 // the check_node.
   double  temp_X = root_X_coord + check_node->get_prelim() + modsum;
   double  temp_Y = root_Y_coord +
                                 (level * tree::get_actual_level_separation());

                                 // Set the x- and y-coordinates of check_node.
   check_node->set_Xcoord(temp_X);
   check_node->set_Ycoord(temp_Y);

                                 // Test whether the x- and y-coordinates
				 // are maximal or minimal. This is needed
				 // in order to adjust a coordinate system to
				 // a drawing of the tree.
   get_max_XYvalue(check_node,x_max,y_max,x_min);

   if (check_node->has_child())
   {
      node *child_ptr = check_node->get_firstchild();
      secondwalk(child_ptr,level+1,
                 modsum + check_node->get_modifier(),x_max,y_max,x_min);
   }
   if (check_node->has_right_sib())
   {
      node *child_ptr = check_node->get_rightsibling();
      secondwalk(child_ptr,level,modsum,x_max,y_max,x_min);
   }
}




/************************************************************************
                      apportion
*************************************************************************/


void   tree::apportion(node & check_node,int level)

   // The subtree of a check_node may still overlay the subtree of its
   // left sibling. In order to find this out, the leftmost descendants
   // of check_node on each level are examined and if necessary, the complete
   // tree is moved to the right.
   // When moving a new subtree further and further to the right gaps may
   // open among smaller subtrees that were previously sandwiched between
   // larger trees. This so called the `left-to-right gluing' problem,
   // which is cleaned up here. When moving a new large subtree to the
   // right, the distance it is moved is also apportioned to smaller interior
   // subtrees.
{

   node *leftmost = check_node.get_firstchild();
   node *neighbor = leftmost->get_leftneighbor();
   int  compare_depth = 1;
 
   while (leftmost != NULL && neighbor != NULL)
				 // Go down the levels of the subtree and
				 // find for every level the leftmost
				 // node of the subtree and its left neighbor.
				 // Then compare the preliminary x-coordinates
				 // of the leftmost and ist left neighbor
				 // and if necessary we "move" the subtree
				 // rooted at check_node by memorizing this
				 // fact in check_nodes modifier field.
   {
                                 // Compute the location of check_node
				 // and where it should be with respect to
				 // the neighbor. 
      double   left_modsum = 0;
      double   right_modsum = 0;
      node    *ancestor_leftmost = leftmost;
      node    *ancestor_neighbor = neighbor;

      for (int i = 0; i < compare_depth; i++)
      {
         ancestor_leftmost = ancestor_leftmost->get_parent();
	 ancestor_neighbor = ancestor_neighbor->get_parent();
	 right_modsum += ancestor_leftmost->get_modifier();
	 left_modsum += ancestor_neighbor->get_modifier();
      }


				 // Find the move_distance and apply it to
			         // check_nodes subtree. Add appropriate
				 // portions to smaller interior subtrees.
      double move_distance = neighbor->get_prelim() + left_modsum +
                            tree::get_subtree_separation() +
			    mean_node_size(leftmost,neighbor) -
			    leftmost->get_prelim() - right_modsum;
			  
      if (move_distance > 0)
      {
                                 // Count interior sibling subtrees in
				 // the left siblings of check_node.
         node *node_ptr = &check_node;
	 int  left_siblings = 0;

	 while (node_ptr != NULL && node_ptr != ancestor_neighbor)
	 {
	    left_siblings++;
	    node_ptr = node_ptr->get_leftsibling();
	 }

	 if (node_ptr != NULL)
	                         // Apply appropriate portions to the subtrees
				 // of the left siblings of check_node.
	 {
	    double portion = move_distance/left_siblings;
	    node_ptr = &check_node;

	    while (node_ptr != ancestor_neighbor)
	    {
	       node_ptr->set_prelim(node_ptr->get_prelim() + move_distance);
	       node_ptr->set_modifier(node_ptr->get_modifier()+move_distance);
	       move_distance -= portion;
	       node_ptr = node_ptr->get_leftsibling();
	    }

	 }

	 else
	 {
	                         // There is nothing to do, since
				 // ancestor_neighbor and ancestor_leftmost
				 // are not siblings of each other.
				 // So moving subtrees has to be done
				 // by an ancestor of check_node.
	 }
      } 
      
                                 // Now get the leftmost descendant of
				 // check_node in the next lower level.
      compare_depth++;
      if (leftmost->is_leaf())
         leftmost = get_left_most(&check_node,0,compare_depth);
      else
         leftmost = leftmost->get_firstchild();
      if (leftmost != NULL)
         neighbor = leftmost->get_leftneighbor();

   }

}

      

/************************************************************************
                     get_left_most
*************************************************************************/


node*  tree::get_left_most(node * check_node,int level,int depth)

   // Returns the leftmost descendant of check_node node at a given depth.
   // This is implemented using a post-order walk of the subtree
   // under check_node, down to the level of depth.
   // Level here is not the absolute tree level used in the two main
   // tree walks; it revers to the level below the node whose leftmost
   // descendant is beeing found. 
{
   if (level >= depth)
      return check_node;

   else if (check_node->is_leaf())
      return NULL;

   else
   {
      node *rightmost = check_node->get_firstchild();
      node *leftmost = get_left_most(rightmost,level+1,depth);

      while (leftmost == NULL && rightmost->has_right_sib())
      {
         rightmost = rightmost->get_rightsibling();
	 leftmost = get_left_most(rightmost,level+1,depth);
      }

      return leftmost;
   }
}
      



/************************************************************************
                  get_max_XYvalue
*************************************************************************/


void tree::get_max_XYvalue(node* node_ptr, double* x_max,
                           double* y_max, double* x_min)

   // Returns the maximal x- and y-coordinates as well as the minimal
   // x-coordinate found until now.
{
   if (node_ptr->get_Xcoord() > (*x_max))
      (*x_max) = node_ptr->get_Xcoord();
   if (node_ptr->get_Ycoord() > (*y_max))
      (*y_max) = node_ptr->get_Ycoord();
   if (node_ptr->get_Xcoord() < (*x_min))
      (*x_min) = node_ptr->get_Xcoord();
}



/************************************************************************
                  deapth_first_search
*************************************************************************/


void   tree::deapth_first_search(node *node_ptr,int level)

   // Recursive deapth-first-search procedure for computing a previous 
   // y-coordinate and determining the number of levels of the tree.
{
   node *child_ptr = NULL;
   
   if (level > level_count)
      level_count = level;
   node_ptr->set_Ycoord(level);
   if (node_ptr->has_child())
   {
      child_ptr = node_ptr->get_firstchild();
      deapth_first_search(child_ptr,level+1);
      while (child_ptr->has_right_sib())
      {
         child_ptr = child_ptr->get_rightsibling();
	 deapth_first_search(child_ptr,level+1);
      }
   }
}




/************************************************************************
                    del_deapth_first
*************************************************************************/


void   tree::del_deapth_first(node *node_ptr)

   // Recursive deapth-first-search procedure for a valid clean_up.
{
    node *child_ptr = NULL;
    node *right_child = NULL;
    

    if (node_ptr->has_child())
    {
       child_ptr = node_ptr->get_firstchild();
       right_child = child_ptr->get_rightsibling();
       del_deapth_first(child_ptr);
       while (right_child != NULL)
       {
          child_ptr = right_child;
	  right_child = child_ptr->get_rightsibling();
	  del_deapth_first(child_ptr);
       }
    }
    delete node_ptr;
}





/************************************************************************
                    mean_node_size
*************************************************************************/


int    tree::mean_node_size(node * left_node,node * right_node)

   // Returns the mean size of the two passed nodes.
   // In this class a trivial calculation, since all nodes 
   // are the same size.
{
   int node_size = 0;

   if (left_node != NULL)
      node_size += left_node->get_radius();
   if (right_node != NULL)
      node_size += right_node->get_radius();

   return node_size;
}




/************************************************************************
                   introduce_child
*************************************************************************/


void  tree::introduce_child(node *tail,node *head)

   // Introduces a new node head as child of its parent tail in
   // the tree.
{
   node *child = NULL;
   node *right_sib = NULL;
   
   if (tail->has_child())
   {
      child = tail->get_firstchild();
      right_sib = child->get_rightsibling();
      if (right_sib != NULL)
      {
         child->set_rightsibling(head);
	 head->set_rightsibling(right_sib);
	 right_sib->set_leftsibling(head);
	 head->set_leftsibling(child);
      }
      else    // right_sib == NULL
      {
         child->set_rightsibling(head);
	 head->set_leftsibling(child);
      }
   }
   else
      tail->set_firstchild(head);
}





