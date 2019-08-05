/*****************************************************************

            Filename    :  node.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/

#include "node.h"
#include <iostream>
#include <string.h>
#include "def.glb"


node::node(int num, int rad, int color, int under_color,
           double xco, double yco, double px, double mx,
           node *par,node *fch,node *lsi,node *rsi,node *lnei)
{
   nmber = num;
   radius = rad;
   displayed = FALSE;
   highlighted = FALSE;
   colour = color;
   underlying_colour = under_color;
   
   x = xco;
   y = yco;
   prelim_x = px;
   mod = mx;


   parent = par;
   firstchild = fch;
   leftsib =lsi;
   rightsib = rsi;
   leftneighb = lnei;
   information[0] = '\0';
   shortinfo[0] = '\0';
}


node::~node()
{

}



node*  node::get_parent() const
{
   return parent;
}

node*  node::get_firstchild() const
{
   return firstchild;
}

node*  node::get_leftsibling() const
{
   return leftsib;
}

node*  node::get_rightsibling() const
{
   return rightsib;
}

node*  node::get_leftneighbor() const
{
   return leftneighb;
}

int   node::get_nmber() const
{
   return nmber;
}

int   node::get_radius() const
{
   return radius;
}

int   node::get_colour() const
{
   return colour;
}

int   node::get_underlying_colour() const
{
   return underlying_colour;
}

double  node::get_Xcoord() const
{
   return x;
}

double  node::get_Ycoord() const
{
   return y;
}

double  node::get_prelim() const
{
   return prelim_x;
}

double  node::get_modifier() const
{
   return mod;
}

char*   node::get_information()
{
   return information;
}

char*   node::get_shortinfo()
{
   return shortinfo;
}

void   node::set_parent(node *put)
{
   parent = put;
}

void   node::set_firstchild(node *put)
{
   firstchild = put;
}

void   node::set_leftsibling(node *put)
{
   leftsib = put;
}

void   node::set_rightsibling(node *put)
{
   rightsib = put;
}

void   node::set_leftneighbor(node *put)
{
   leftneighb = put;
}

void  node::set_nmber(int m)
{
   nmber = m;
}

void  node::set_colour(int m)
{
   colour = m;
}

void  node::set_underlying_colour(int m)
{
   underlying_colour = m;
}

void  node::set_Xcoord(double m)
{
   x = m;
}

void  node::set_Ycoord(double m)
{
   y = m;
}

void  node::set_prelim(double m)
{
   prelim_x = m;
}

void  node::set_modifier(double m)
{
   mod = m;
}

void  node::set_information(char* info)
{
   strcpy(information,info);
}

void  node::set_shortinfo(char* info)
{
   strcpy(shortinfo,info);
}

void  node::add_information(char* info)
{
   strcat(information,info);
}

void  node::add_shortinfo(char* info)
{
   strcat(shortinfo,info);
}

int   node::is_leaf() const
{
   return ( firstchild == NULL ? TRUE : FALSE );
}

int   node::has_left_sib() const
{
   return ( leftsib != NULL ? TRUE : FALSE );
}

int   node::has_right_sib() const
{
   return ( rightsib != NULL ? TRUE : FALSE );
}

int   node::has_child() const
{
   return ( firstchild != NULL ? TRUE : FALSE);
}


int   node::is_displayed()const
{
   return displayed;
}

void  node::display(int number)
{
   displayed = number;
}


int   node::is_highlighted()const
{
   return highlighted;
}

void  node::highlight(int boolean)
{
   highlighted = boolean;
}


std::ostream &operator<<(std::ostream & output, const node &n)
{
   output << "x = " << n.x << std::endl
          << "y = " << n.y << std::endl
	  << "prelim_x = " << n.prelim_x << std::endl
	  << "mod = " << n.mod << std::endl
          << "parent = " << ((n.parent != NULL) ? n.parent : 0) << std::endl
          << "firstchild = " << ((n.firstchild != NULL) ? n.firstchild : 0) << std::endl
          << "leftsib = " << ((n.leftsib != NULL) ? n.leftsib : 0) << std::endl
          << "rightsib = " << ((n.rightsib != NULL) ? n.rightsib : 0) << std::endl
          << "leftneighb = " << ((n.leftneighb != NULL) ? n.leftneighb : 0) << std::endl;

   return output;
}
