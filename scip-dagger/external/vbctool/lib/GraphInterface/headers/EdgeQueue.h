/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      EdgeQueue.h
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
#ifndef EDGEQUEUE_H
#define EDGEQUEUE_H

#include "GFE.inc"
#include "DrawArea.h"
#include "List.h"
#include "Timer.h"

class Edge2;
class EdgeCategorie;

class EdgeQueue : Timer
{
   private:

      Queue<Edge2*> *_edgeQueue;
      Queue<Edge2*> *_tmpQueue;

      static void drawEdgesCallback(Rect<CoordType> *r, XtPointer);
      void drawEdges(Rect<CoordType> *r);

      Boolean action();

   public:

      EdgeQueue();
      ~EdgeQueue();

      void drawEdge(int ec, CoordType, CoordType, CoordType, CoordType);
      void deleteQueue(Boolean);

      void enableCallbacks()  { theDrawArea->addRepaintCallback(clientRepaintCallback(drawEdgesCallback), XtPointer(this)); }
      void disableCallbacks() { theDrawArea->removeRepaintCallback(clientRepaintCallback(drawEdgesCallback), XtPointer(this)); }
};

#endif
