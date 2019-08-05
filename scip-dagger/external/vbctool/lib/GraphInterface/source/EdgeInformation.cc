
#include "EdgeInformation.h"
#include "DrawArea.h"
#include "NodeInformation.h"
#include "GFEResourceManager.h"
#include "GraphInterface.h"
#include <assert.h>

EdgeInformation::EdgeInformation()
{
   if (!_edgeCategorie) {
      _edgeCategorie = theGraphInterface->edgeCategorie();
      _maxcat = theGraphInterface->maxEdgeCategories();
   }

   _nodeInformation = NULL;
   _adjazentList = NULL;
}

EdgeInformation::~EdgeInformation()
{
   if (_adjazentList) {
      Stack<Edge*> *stack = _adjazentList->getStack();
      Edge* edge;
      while(edge = stack->pop())
	 delete edge;
      delete stack;
   
      delete _adjazentList;
   }

   disableCallback();
}

void EdgeInformation::draw(int cat, int from, int to)
{
   register int mn = maxNodes();
   if(cat < 1 || cat > _maxcat ||
      from == to                ||
      from < 1 || to < 1        ||
      from > mn || to > mn) return;

   Edge *edge;

   if (edge = _adjazentList->find(from, to))
      edge->_categorie = _edgeCategorie[cat];
   else {
      edge = new Edge(from, to, _edgeCategorie[cat]);
      assert(_adjazentList->insertContent(from, edge, to) == SUCCESS);
   }
   
   edge->draw(_nodeInformation);
}

void EdgeInformation::erase(int from, int to)
{
   register int mn = maxNodes();
   if(from == to                ||
      from < 1 || to < 1        ||
      from > mn || to > mn) return;

   Edge *edge;
   if (edge = _adjazentList->find(from, to)) {
      edge->erase(_nodeInformation);
      _adjazentList->deleteContent(from, edge, to);
      delete edge;
   }
}

void EdgeInformation::mxor(int from, int to)
{
   register int mn = maxNodes();
   if(from == to                ||
      from < 1 || to < 1        ||
      from > mn || to > mn) return;

   Edge *edge;
   if (edge = _adjazentList->find(from, to)) edge->mxor(_nodeInformation);
}

void EdgeInformation::drawEdges(Rect<CoordType> *r)
{
   if (_adjazentList) {
      Stack<Edge*> *stack = _adjazentList->getStack();
      Edge* edge;
      while(edge = stack->pop())
	 if (isIn(edge, r))
	    edge->draw(_nodeInformation);
      delete stack;
   }
}

void EdgeInformation::drawEdgesCallback(Rect<CoordType> *r, XtPointer p)
{
   EdgeInformation *instance = (EdgeInformation *) p;
   instance->drawEdges(r);
}

void EdgeInformation::registerNodeInformation(NodeInformation* ni)
{
   _nodeInformation = ni;
   enableCallback();
}

Boolean EdgeInformation::isIn(Edge *e, Rect<CoordType> *rect)
{
   if (!e) return False;
   Line<CoordType> l(_nodeInformation->getNode(e->from())->x(),
		     _nodeInformation->getNode(e->from())->y(),
		     _nodeInformation->getNode(e->to())->x(),
		     _nodeInformation->getNode(e->to())->y());

   for (int j=0; j<=3; j++)
   {
      if (rect[j].isEmpty())
	 break;

      if (l/rect[j]) return True;
   }

   return False;
}


void EdgeInformation::registerAdjazentList(int n)
{
   if (!_adjazentList) {
      initAdjazentList(n);
      return;
   }

   int oldNum = _adjazentList->numElements();
   if (n == oldNum) return;
   
   AdjazentList<Edge*> *newlist = new AdjazentList<Edge*>(n, _adjazentList->directed());
   Stack<Edge*> *stack = _adjazentList->getStack();
   Edge *edge;

   while(edge = stack->pop()) {
      if (edge->from() > n || edge->to() > n) {
	 delete edge;
	 continue;
      }
      newlist->insertContent(edge->from(), edge, edge->to());
   }

   delete stack;
   delete _adjazentList;
   _adjazentList = newlist;
}
