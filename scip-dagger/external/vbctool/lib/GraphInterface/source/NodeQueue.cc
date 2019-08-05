/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      NodeQueue.cc
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


#include "NodeQueue.h"
#include "NodeEdge.h"
#include "GraphInterface.h"

NodeQueue::NodeQueue() : Timer(3)
{
   _nodeQueue = NULL;
   _tmpQueue = NULL;
}

NodeQueue::~NodeQueue()
{
   deleteQueue(FALSE);
}


void NodeQueue::drawNode(int nr, int nc, CoordType x, CoordType y)
{
   if (nc < 1 || nc > theGraphInterface->maxNodeCategories()) return;
   
   NodeCategorie *nodecat = theGraphInterface->nodeCategorie()[nc];
   
   if (_tmpQueue == NULL) {
      _tmpQueue = new Queue<Node*>;
      enableCallbacks();
   }

   if (_tmpQueue->front() == NULL) // if tmp-Queue isn't empty, action is already initialized
      start();
   
   Node *n = new Node(nr, x, y, nodecat);
   n->draw();
   _tmpQueue->enqueue(n);
}

void NodeQueue::deleteQueue(Boolean clearOnScreen)
{
   if (_nodeQueue) {
      while (!_nodeQueue->isEmpty()) {
	 Node *n = _nodeQueue->dequeue();
	 if (clearOnScreen) n->erase();
	 delete n;
      }
      delete _nodeQueue;
      _nodeQueue = NULL;
   }

   if (_tmpQueue) {
      while (!_tmpQueue->isEmpty()) {
	 Node *n = _tmpQueue->dequeue();
	 if (clearOnScreen) n->erase();
	 delete n;
      }
      delete _tmpQueue;
      _tmpQueue = NULL;
   }

   disableCallbacks();
}

void NodeQueue::drawNodesCallback(Rect<CoordType> *r, XtPointer p)
{
   NodeQueue *instance = (NodeQueue*) p;
   instance->drawNodes(r);
}

void NodeQueue::drawNodes(Rect<CoordType> *rect)
{
   static Rect<CoordType> r;
   static CoordType rr;
   static CoordType radius;

   if (_nodeQueue) {
      Element<Node*> *n = _nodeQueue->front();

      while(n) {

	 radius = n->item->_categorie->_radius;
	 r.newInit(n->item->_x - radius, n->item->_y - radius,
		   rr = 2.0 * radius, rr);

	 for (int j = 0; j <= 3; j++)
	    if (rect[j].isEmpty()) break;
	    else
	       if (r / rect[j])
		  n->item->draw();

	 n = n->next;
      }
   }

   Element<Node*> *n = _tmpQueue->front();

   while(n) {

      radius = n->item->_categorie->_radius;
      r.newInit(n->item->_x - radius, n->item->_y - radius,
		rr = 2.0 * radius, rr);

      for (int j = 0; j <= 3; j++)
	 if (rect[j].isEmpty()) break;
	 else
	    if (r / rect[j])
	       n->item->draw();

      n = n->next;
   }

}

Boolean NodeQueue::action()
{
   if (_tmpQueue->isEmpty()) {
      //fprintf(stderr, "tmp-Queue empty\n");
      return False;
   }

   if (_nodeQueue == NULL)
      _nodeQueue = new Queue<Node*>;

   static Element<Node*> *h = NULL;
   static Node *n = NULL;

   int count = 0;

   if (n == NULL) {
      n = _tmpQueue->dequeue();
      h = _nodeQueue->front();
   }
   
   while (h) {
      Node *hn = h->item;
      if (hn->_x == n->_x &&
	  hn->_y == n->_y &&
	  hn->_number == n->_number) {
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
   _nodeQueue->enqueue(n);
   n = NULL;

   return True;
}
