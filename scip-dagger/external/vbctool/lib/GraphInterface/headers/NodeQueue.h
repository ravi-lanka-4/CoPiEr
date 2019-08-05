/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeQueue.h
//
//      Version  :      12.05.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////
#ifndef NODEQUEUE_H
#define NODEQUEUE_H

#include "GFE.inc"
#include "DrawArea.h"
#include "List.h"
#include "Timer.h"

class Node;
class NodeCategorie;

class NodeQueue : Timer
{
   private:

      Queue<Node*> *_nodeQueue;
      Queue<Node*> *_tmpQueue;

      static void drawNodesCallback(Rect<CoordType> *r, XtPointer);
      void drawNodes(Rect<CoordType> *r);

      Boolean action();

   public:

      NodeQueue();
      ~NodeQueue();

      void drawNode(int, int nc, CoordType, CoordType);
      void deleteQueue(Boolean);

      void enableCallbacks()  { theDrawArea->addRepaintCallback(clientRepaintCallback(drawNodesCallback), XtPointer(this)); }
      void disableCallbacks() { theDrawArea->removeRepaintCallback(clientRepaintCallback(drawNodesCallback), XtPointer(this)); }
};

#endif
