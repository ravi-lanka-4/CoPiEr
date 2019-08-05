#ifndef EDGEINFORMATION_H
#define EDGEINFORMATION_H

#include "GFE.inc"
#include "List.h"
#include "NodeEdge.h"
#include "DrawArea.h"

class NodeInformation;

class EdgeInformation
{
   private:

      NodeInformation	      *_nodeInformation;

      static int	       _maxcat;
      static EdgeCategorie   **_edgeCategorie;

      AdjazentList<Edge*>     *_adjazentList;

      static void drawEdgesCallback(Rect<CoordType> *r, XtPointer);
      void drawEdges(Rect<CoordType> *r);

   public:

      EdgeInformation();
      ~EdgeInformation();

      void draw(int, int, int);
      void erase(int, int);
      void mxor(int, int);

      void registerNodeInformation(NodeInformation* ni);

      void initAdjazentList(int n, Boolean directed = False)
      {
	 if (_adjazentList) {
	    Stack<Edge*> *stack = _adjazentList->getStack();
	    Edge* edge;
	    while(edge = stack->pop())
	       delete edge;

	    delete stack;
	    delete _adjazentList;
	 }
	 _adjazentList = new AdjazentList<Edge*>(n, directed);
      }

      void registerAdjazentList(int n);

      int maxNodes()
      {
	 if (_adjazentList)
	    return _adjazentList->numElements();
	 else return -1;
      }

      Boolean isIn(Edge*, Rect<CoordType> *);
      EdgeCategorie *getCategorie(int i) { return _edgeCategorie[i]; }

      void enableCallback()  { theDrawArea->addRepaintCallback(clientRepaintCallback(drawEdgesCallback), XtPointer(this)); }
      void disableCallback() { theDrawArea->removeRepaintCallback(clientRepaintCallback(drawEdgesCallback), XtPointer(this)); }
};

#endif
