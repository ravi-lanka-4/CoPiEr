
#include "NodeInformation.h"
#include "DrawArea.h"
#include "Context.h"
#include "GFEResourceManager.h"
#include "ColorInformation.h"
#include "GraphInterface.h"
#include "EdgeInformation.h"
#include <limits.h>
#include <assert.h>

NodeInformation::NodeInformation(EdgeInformation* ei)
{
   if (!_nodeCategorie) {
      _nodeCategorie = theGraphInterface->nodeCategorie();
      _maxcat = theGraphInterface->maxNodeCategories();
   }

   _edgeInformation = ei;
   
   _maxnodes = 0;
   _nodeArray = NULL;

   enableCallback();
}

NodeInformation::~NodeInformation()
{
   if (_nodeArray) {
      for (int i = 1; i <= _maxnodes; i++)
	 delete _nodeArray[i];

      delete[] _nodeArray;
   }

   disableCallback();
}

void NodeInformation::initNodes(int n)
{
   if (_nodeArray) {
      for (int i = 1; i <= _maxnodes; i++)
	 delete _nodeArray[i];

      delete[] _nodeArray;
   }

   _edgeInformation->initAdjazentList(n);

   _maxnodes = n;
   _nodeArray = new Node*[n+1];

   for (int i = 0; i <= n; i++)
      _nodeArray[i] = NULL;
}

void NodeInformation::registerNodes(int n)
{
   if (!_nodeArray){
      initNodes(n);
      return;
   }

   Node **newArray = new Node*[n+1];

   for (int i = 1; i <= _maxnodes; i++)
      if (i <= n)
	 newArray[i] = _nodeArray[i];
      else delete _nodeArray[i];

   for (int i = _maxnodes + 1; i <= n; i++)
      newArray[i] = NULL;

   delete[] _nodeArray;

   _maxnodes = n;
   _nodeArray = newArray;

   _edgeInformation->registerAdjazentList(n);
}

void NodeInformation::registerNode(int cat, int nr, CoordType x, CoordType y,
				   Boolean drawn, int other)
{
   if (nr < 1 || nr > _maxnodes ||
       cat < 1 || cat > _maxcat) return;

   NodeCategorie *otherCategorie = NULL;
   if (other >= 1 && other <=_maxcat) otherCategorie = _nodeCategorie[other];
   
   delete _nodeArray[nr];

   _nodeArray[nr] = new Node(nr, x, y, _nodeCategorie[cat], drawn, otherCategorie);
}

void NodeInformation::draw(int cat, int nr, int other)
{
   if (cat < 1 || cat > _maxcat ||
       nr < 1 || nr > _maxnodes  ||
       !_nodeArray[nr]) return;

   NodeCategorie *otherCategorie = NULL;
   if (other >= 1 && other <=_maxcat) otherCategorie = _nodeCategorie[other];

   Node *n;
   (n = _nodeArray[nr])->_categorie = _nodeCategorie[cat];
   n->_other = otherCategorie;
   n->_drawn = True;
   n->draw();
}

void NodeInformation::draw(int nr)
{
   if (nr < 1 || nr > _maxnodes || !_nodeArray[nr]) return;

   _nodeArray[nr]->draw();
}

void NodeInformation::erase(int nr)
{
   if (nr < 1 || nr > _maxnodes || !_nodeArray[nr]) return;

   Node *n;
   (n = _nodeArray[nr])->erase();
   n->_drawn = False;
}

void NodeInformation::mxor(int nr)
{
   if (nr < 1 || nr > _maxnodes || !_nodeArray[nr]) return;

   _nodeArray[nr]->mxor();
}

void NodeInformation::drawNodesCallback(Rect<CoordType> *r, XtPointer p)
{
   NodeInformation *instance = (NodeInformation *) p;
   instance->repaint(r);
}

void NodeInformation::repaint(Rect<CoordType> *rect)
{
   static Rect<CoordType> r;
   static CoordType rr;
   static CoordType radius;

   if (!_nodeArray) return;
   
   for (int i = 1; i <= _maxnodes; i++) {

      if (!_nodeArray[i] || !_nodeArray[i]->_drawn) continue;

      radius = _nodeArray[i]->_categorie->_radius;
      r.newInit(_nodeArray[i]->_x - radius, _nodeArray[i]->_y - radius,
		rr = 2.0 * radius, rr);

      for (int j = 0; j <= 3; j++)
	 if (rect[j].isEmpty()) break;
	 else
	    if (r / rect[j]) {
	       _nodeArray[i]->draw();
	       break;
	    }
   }
}

int NodeInformation::nextNode(Point<CoordType> *point)
{
   double distance = DBL_MAX;
   int node = -1;

   for (int i = 1; i <= _maxnodes; i++) {
      if (!_nodeArray[i]) continue;

      CoordType ax = _nodeArray[i]->_x;
      CoordType ay = _nodeArray[i]->_y;

      ax -= point->x();
      ay -= point->y();

      double l = (ax * ax) + (ay * ay);
      if (l<distance)
      {
	 distance = l;
	 node = i;
      }
   }

   return node;
}

int NodeInformation::getNode(Point<CoordType> *point)
{
   for (int i = 1; i <= _maxnodes; i++) {
      if (!_nodeArray[i]) continue;

      if (_nodeArray[i]->_x == point->x() &&
	  _nodeArray[i]->_y == point->y()) return i;
   }

   return -1;
}
