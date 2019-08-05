/*****************************************************************

            Filename    :  edge.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Class for managing edges.
	    
******************************************************************/


#ifndef EDGE_H
#define EDGE_H

#include <iostream>
#include "def.glb"


class edge {

public:

   edge(int = 0, int = 0);
   ~edge();

   int  get_first();
   int  get_second();

   void set_first(int f);
   void set_second(int s);

private:

   int  first;
   int  second;

};



#endif
