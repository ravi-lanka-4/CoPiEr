/*****************************************************************

            Filename    :  node.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Class for managing nodes.
	    
******************************************************************/


#ifndef NODE_H
#define NODE_H

#include <iostream>
#include "def.glb"


class node {
   friend std::ostream &operator<<(std::ostream &, const node &);
public:
   node(int = 0, int = 1, int = 1,int = 1,
        double = 0, double = 0, double = 0, double = 0,
        node * = 0,node * = 0,node * = 0,node * = 0,node * = 0);
   ~node();

   node*   get_parent() const;
   node*   get_firstchild() const;
   node*   get_leftsibling() const;
   node*   get_rightsibling() const;
   node*   get_leftneighbor() const;
   int     get_nmber() const;
   int     get_radius() const;
   int     get_colour() const;
   int     get_underlying_colour() const;
   double  get_Xcoord() const;
   double  get_Ycoord() const;
   double  get_prelim() const;
   double  get_modifier() const;
   char*   get_information();
   char*   get_shortinfo();

   void    set_parent(node *);
   void    set_firstchild(node *);
   void    set_leftsibling(node *);
   void    set_rightsibling(node *);
   void    set_leftneighbor(node *);
   void    set_nmber(int);
   void    set_colour(int);
   void    set_underlying_colour(int);
   void    set_Xcoord(double);
   void    set_Ycoord(double);
   void    set_prelim(double);
   void    set_modifier(double);
   void    set_information(char*);
   void    set_shortinfo(char*);

   void    add_information(char*);
   void    add_shortinfo(char*);

   int     is_leaf() const;
   int     has_left_sib() const;
   int     has_right_sib() const;
   int     has_child() const;
   
   int     is_displayed() const;
   void    display(int);

   int     is_highlighted() const;
   void    highlight(int);

private:

   int     nmber;
   int     radius;
   int     displayed;
   int     highlighted;
   int     colour;
   int     underlying_colour;
   
   double  x;
   double  y;
   double  prelim_x;
   double  mod;

   char    information[INFOSIZE];
   char    shortinfo[SHORTSIZE];
   
   node*   parent;
   node*   firstchild;
   node*   leftsib;
   node*   rightsib;
   node*   leftneighb;
   

};


#endif
