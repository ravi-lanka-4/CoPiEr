/*****************************************************************

            Filename    :  basic_tree.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.

******************************************************************/


#include "basic_tree.h"
#include <iostream>
#include <string.h>





/************************************************************************
                         PUBLIC

             This part contains all public files
*************************************************************************/



/************************************************************************
                         constructor
*************************************************************************/


basic_tree::basic_tree()

{
   node_nb = 0;
   edge_nb = 0;

   standard = FALSE;
   
   root = NULL;
   
   edge_array = NULL;
   node_array = NULL;

   array_size = 0;

}



/************************************************************************
                         destructor
*************************************************************************/


basic_tree::~basic_tree()
{
   int i = 0;
   delete[] edge_array;
   for ( i = 1; i <= (array_size == 0 ? node_nb : array_size); i++)
   {
      delete node_array[i];
   }
   delete[] node_array;
}





/************************************************************************
                      getnode_edge_first
*************************************************************************/


int   basic_tree::getnode_edge_first(int i) const

   // Returns the tail of the edge i. 
{
   if (i <= edge_nb && i > 0)
      return edge_array[i].get_first();
   else
   {
      std::cout << "tree::getnode_edge_first: wrong access" << std::endl;
      return 0;
   }
}



/************************************************************************
                      getnode_edge_sec
*************************************************************************/


int   basic_tree::getnode_edge_sec(int i) const

   // Returns the head of the edge i. 
{
   if (i <= edge_nb && i > 0)
      return edge_array[i].get_second();
   else
   {
      std::cout << "tree::getnode_edge_sec: wrong access" << std::endl;
      return 0;
   }
}




/************************************************************************
                      get_node_nb
*************************************************************************/


int  basic_tree::get_node_nb() const

   // Returns the number of nodes in the tree.
{
   return node_nb;
}





/************************************************************************
                        get_Xcoord
*************************************************************************/


double  basic_tree::get_Xcoord(int i) const

   // Returns the x-coordinate of node i.
{
   if (i <= node_nb && i > 0)
      return node_array[i]->get_Xcoord();
   else
   {
      std::cout << "basic_tree::get_Xcoord: wrong acces"
           << " at node " << i << std::endl;
      return 0;
   }
}

   

/************************************************************************
                        get_Ycoord
*************************************************************************/


double  basic_tree::get_Ycoord(int i) const

    // Returns the y-coordinate of node i.
{
   if (i <= node_nb && i > 0)
      return node_array[i]->get_Ycoord();
   else
   {
      std::cout << "basic_tree::get_Ycoord: wrong acces"
           << " at node " << i << std::endl;
      return 0;
   }

}





/************************************************************************
                     get_node_info
*************************************************************************/


char* basic_tree::get_node_info(int i) const

    // Returns the information stored by node i.
{
   if (i <= node_nb && i > 0)
      return node_array[i]->get_information();
   else
   {
      std::cout << "basic_tree::get_node_info: wrong access" 
           << " at node " << i << std::endl;
      return NULL;
   }
}




/************************************************************************
                     get_node_short
*************************************************************************/


char* basic_tree::get_node_short(int i) const

    // Returns the short information stored by node i.
{
   if (i <= node_nb && i > 0)
      return node_array[i]->get_shortinfo();
   else
   {
      std::cout << "basic_tree::get_node_short: wrong access"
           << " at node " << i << std::endl;
      return NULL;
   }
}




/************************************************************************
                     get_node_colour
*************************************************************************/


int  basic_tree::get_node_colour(int node) const

    // Returns the colour stored by node.
{
   if (node <= node_nb && node > 0)
      return node_array[node]->get_colour();
   else
   {
      std::cout << "basic_tree::get_node_colour: wrong access"
           << " at node " << node << std::endl;
      return 0;
   }

}




/************************************************************************
                     get_under_colour
*************************************************************************/


int  basic_tree::get_under_colour(int node) const

    // Returns the underlying colour stored by node.
{
   if (node <= node_nb && node > 0)
      return node_array[node]->get_underlying_colour();
   else
   {
      std::cout << "basic_tree::get_under_colour: wrong access"
           << " at node " << node << std::endl;
      return 0;
   }
}




/************************************************************************
                      is_standard
*************************************************************************/


int  basic_tree::is_standard() const

   // Returns whether the tree contains standard information.
{
   return standard;
}



/************************************************************************
                     is_displayed
*************************************************************************/


int  basic_tree::is_displayed(int node) const

    // Returns if the node is displayed.
{
   if (node <= node_nb && node > 0)
      return node_array[node]->is_displayed();
   else
   {
      std::cout << "basic_tree::is_displayed: wrong access"
           << " at node " << node << std::endl;
      return 0;
   }

}




/************************************************************************
                     is_highlighted
*************************************************************************/


int  basic_tree::is_highlighted(int node) const

    // Returns if the node i is highlighted.
{
   if (node <= node_nb && node > 0)
      return node_array[node]->is_highlighted();
   else
   {
      std::cout << "basic_tree::is_highlighted: wrong access"
           << " at node " << node << std::endl;
      return 0;
   }
}





/************************************************************************
                      set_standard
*************************************************************************/


void  basic_tree::set_standard(int t)

   // Sets whether the tree contains standard information.
{
   standard = t;
}




/************************************************************************
                     set_node_colour
*************************************************************************/


void  basic_tree::set_node_colour(int node,int colour)

   // Set the colour of a node.
{ 
   if (node <= node_nb && node > 0)
      node_array[node]->set_colour(colour);
   else
      std::cout << "basic_tree::set_node_colour: wrong access"
           << " at node " << node << std::endl;
}




/************************************************************************
                     set_under_colour
*************************************************************************/


void basic_tree::set_under_colour(int node,int colour)

   // Set the underlying colour of a node.
{ 
   if (node <= node_nb && node > 0)
      node_array[node]->set_underlying_colour(colour);
   else
      std::cout << "basic_tree::set_under_colour: wrong access"
           << " at node " << node << std::endl;
}




/************************************************************************
                        display
*************************************************************************/


void  basic_tree::display(int node,int number)

   // Set if the node is displayed.
{ 
   if (node <= node_nb && node > 0)
      node_array[node]->display(number);
   else
      std::cout << "basic_tree::display: wrong access"
           << " at node " << node << std::endl;
      
}





/************************************************************************
                     highlight
*************************************************************************/


void basic_tree::highlight(int node,int number)

   // Set if the node is highlighted.
{ 
   if (node <= node_nb && node > 0)
      node_array[node]->highlight(number);
   else
      std::cout << "basic_tree::highlight: wrong access"
           << " at node " << node << std::endl;
}




/************************************************************************
                     tree_empty
*************************************************************************/


int  basic_tree::tree_empty()

   // Returns if the root is empty or not.
{
//   std::cout << "tree_empty: Root : " << root->get_nmber() << std::endl;
   if (root == NULL)
      return TRUE;
   else
      return FALSE;
}
