/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Graph.cc
//
//      Version  :      15.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////


#include "Graph.h"

Graph::Graph(int n)
{
   _ei = new EdgeInformation();
   _ni = new NodeInformation(_ei);
   _ei->registerNodeInformation(_ni);

   if (n >= 0) initNodes(n);
}

Graph::~Graph()
{
   delete _ni;
   delete _ei;
}
