/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Graph.h
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
#ifndef GRAPH_H
#define GRAPH_H

#include "EdgeInformation.h"
#include "NodeInformation.h"

class Graph
{
   private:

      NodeInformation	*_ni;
      EdgeInformation	*_ei;

   protected:

   public:

      Graph(int n = -1);
      ~Graph();

      void initNodes(int n) { _ni->initNodes(n); }
      void registerNodes(int n) { _ni->registerNodes(n); }
      void registerNode(int c, int n, CoordType x, CoordType y,
			Boolean drawn = False, int other = -1)
			{ _ni->registerNode(c, n, x, y, drawn, other); }

      void drawNode(int c, int n, int other = -1)
		    { _ni->draw(c, n, other); }
      void drawNode(int n) { _ni->draw(n); }
      void eraseNode(int n) { _ni->erase(n); }
      void xorNode(int n) { _ni->mxor(n); }

      int maxNodes() { return _ni->maxNodes(); }
      Node *getNode(int i) { return _ni->getNode(i); }
      int nextNode(Point<CoordType> *p) { return _ni->nextNode(p); }
      int getNode(Point<CoordType> *p) { return _ni->getNode(p); }

      void setRadius(int c, CoordType v) { _ni->setRadius(c, v); }



      void drawEdge(int c, int f, int t) { _ei->draw(c, f, t); }
      void eraseEdge(int f, int t) { _ei->erase(f, t); }
      void xorEdge(int f, int t) { _ei->mxor(f, t); }

      void initAdjazentList(int n, Boolean directed = False)
	 { _ei->initAdjazentList(n, directed); }

      void registerAdjazentList(int n)
	 { _ei->registerAdjazentList(n); }

      NodeInformation *nodeInformation() { return _ni; }
      EdgeInformation *edgeInformation() { return _ei; }

      void enableNodesCallback()  { _ni->enableCallback(); }
      void disableNodesCallback() { _ni->disableCallback(); }
      void enableEdgesCallback()  { _ei->enableCallback(); }
      void disableEdgesCallback() { _ei->disableCallback(); }

      void enableGraphCallbacks()
      {
	 enableNodesCallback();
	 enableEdgesCallback();
      }
      void disableGraphCallbacks()
      {
	 disableNodesCallback();
	 disableEdgesCallback();
      }
};

#endif
