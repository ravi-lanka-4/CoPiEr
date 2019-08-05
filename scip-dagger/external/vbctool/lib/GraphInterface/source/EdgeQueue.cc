/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      EdgeQueue.cc
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


#include "EdgeQueue.h"
#include "NodeEdge.h"
#include "GraphInterface.h"

EdgeQueue::EdgeQueue() : Timer(3)
{
   _edgeQueue = NULL;
   _tmpQueue = NULL;
}

EdgeQueue::~EdgeQueue()
{
   deleteQueue(FALSE);
}


void EdgeQueue::drawEdge(int ec, CoordType x1, CoordType y1, CoordType x2, CoordType y2)
{
   if (ec < 1 || ec > theGraphInterface->maxEdgeCategories()) return;
   
   EdgeCategorie *edgecat = theGraphInterface->edgeCategorie()[ec];
   
   if (_tmpQueue == NULL) {
      _tmpQueue = new Queue<Edge2*>;
      enableCallbacks();
   }

   if (_tmpQueue->front() == NULL) // if tmp-Queue isn't empty, action is already initialized
      start();
   
   Edge2 *n = new Edge2(x1, y1, x2, y2, edgecat);
   n->draw();
   _tmpQueue->enqueue(n);
}

void EdgeQueue::deleteQueue(Boolean clearOnScreen)
{
   if (_edgeQueue) {
      while (!_edgeQueue->isEmpty()) {
	 Edge2 *n = _edgeQueue->dequeue();
	 if (clearOnScreen) n->erase();
	 delete n;
      }
      delete _edgeQueue;
      _edgeQueue = NULL;
   }

   if (_tmpQueue) {
      while (!_tmpQueue->isEmpty()) {
	 Edge2 *n = _tmpQueue->dequeue();
	 if (clearOnScreen) n->erase();
	 delete n;
      }
      delete _tmpQueue;
      _tmpQueue = NULL;
   }

   disableCallbacks();
}

void EdgeQueue::drawEdgesCallback(Rect<CoordType> *r, XtPointer p)
{
   EdgeQueue *instance = (EdgeQueue*) p;
   instance->drawEdges(r);
}

void EdgeQueue::drawEdges(Rect<CoordType> *rect)
{
   if (_edgeQueue) {
      Element<Edge2*> *n = _edgeQueue->front();

      while(n) {

	 Line<CoordType> l(n->item->x1(),
			   n->item->y1(),
			   n->item->x2(),
			   n->item->y2());

	 for (int j = 0; j <= 3; j++)
	    if (rect[j].isEmpty()) break;
	    else
	       if (l / rect[j])
		  n->item->draw();

	 n = n->next;
      }
   }

   Element<Edge2*> *n = _tmpQueue->front();

   while(n) {

      Line<CoordType> l(n->item->x1(),
			n->item->y1(),
			n->item->x2(),
			n->item->y2());

      for (int j = 0; j <= 3; j++)
	 if (rect[j].isEmpty()) break;
	 else
	    if (l / rect[j])
	       n->item->draw();

      n = n->next;
   }

}

Boolean EdgeQueue::action()
{
   if (_tmpQueue->isEmpty()) {
      //fprintf(stderr, "tmp-Queue empty\n");
      return False;
   }

   if (_edgeQueue == NULL)
      _edgeQueue = new Queue<Edge2*>;

   static Element<Edge2*> *h = NULL;
   static Edge2 *n = NULL;

   int count = 0;

   if (n == NULL) {
      n = _tmpQueue->dequeue();
      h = _edgeQueue->front();
   }
   
   while (h) {
      Edge2 *hn = h->item;
      if (hn->x1() == n->x1() &&
	  hn->y1() == n->y1() &&
	  hn->x2() == n->x2() &&
	  hn->y2() == n->y2()) {
	    h->item = n;
	    delete hn;
	    n = NULL;
	    //fprintf(stderr, "-");
	    return True;
      }

      h = h->next;
      if(++count > 1000 && h) return True; // not more then 1000 tries
   }

   //fprintf(stderr, "*");
   _edgeQueue->enqueue(n);
   n = NULL;

   return True;
}
