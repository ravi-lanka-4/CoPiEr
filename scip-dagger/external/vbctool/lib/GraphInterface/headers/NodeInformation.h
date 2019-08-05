#ifndef NODEINFORMATION_H
#define NODEINFORMATION_H

#include "GFE.inc"
#include "NodeEdge.h"
#include "DrawArea.h"

class Node;
class EdgeInformation;

class NodeInformation
{
   private:

      EdgeInformation *_edgeInformation;

      Node	     **_nodeArray;
      int	       _maxnodes;

      static NodeCategorie  **_nodeCategorie;
      static int	      _maxcat;

      static void drawNodesCallback(Rect<CoordType> *r, XtPointer);
      void repaint(Rect<CoordType> *r);

   public:

      NodeInformation(EdgeInformation*);
      ~NodeInformation();

      void initNodes(int n);
      void registerNodes(int n);
      void registerNode(int, int, CoordType, CoordType, Boolean drawn = False, int other = -1);

      void draw(int cat, int nr, int other = -1);
      void draw(int nr);
      void erase(int nr);
      void mxor(int nr);

      int maxNodes() { return _maxnodes; }
      Node *getNode(int i) { return _nodeArray[i]; }
      int   nextNode(Point<CoordType>*);
      int   getNode(Point<CoordType>*);

      int maxCategories() { return _maxcat; }
      NodeCategorie *getCategorie(int i) { return _nodeCategorie[i]; }

      void setRadius(int cat, CoordType v) { _nodeCategorie[cat]->_radius = v; }

      void enableCallback()  { theDrawArea->addRepaintCallback(clientRepaintCallback(drawNodesCallback), XtPointer(this)); }
      void disableCallback() { theDrawArea->removeRepaintCallback(clientRepaintCallback(drawNodesCallback), XtPointer(this)); }
};

#endif
